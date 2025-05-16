/* serveur.c
 * Jean-Samuel PIERRE 12201116
 * Je déclare qu'il s'agit de mon propre travail.
 */

#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/list.h"
#include "../include/user.h"
#include "../include/utils.h"

#define PORT_FREESCORD 4321

int tube[2];
struct list *list_user = NULL;
pthread_t repeteur;
int last;

// Crée la socket serveur et lance le listen
int create_listening_sock(uint16_t port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return -1;
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)};
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sock);
        return -1;
    }
    if (listen(sock, 10) < 0)
    {
        perror("listen");
        close(sock);
        return -1;
    }
    return sock;
}

// Thread de rebroadcast
void *fonc_thread(void *arg)
{
    (void)arg;
    char buf[512];
    while (1)
    {
        ssize_t n = read(tube[0], buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            for (int i = 0; i < list_length(list_user); ++i)
            {
                struct user *tmp = list_get(list_user, i);
                if (tmp->sock != last)
                {
                    send(tmp->sock, buf, n, 0);
                }
            }
        }
    }
    return NULL;
}

// Thread de gestion d’un client
void *handle_client(void *arg)
{
    struct user *iencli = arg;
    list_add(list_user, iencli);

    char chaine[512];
    // Boucle normale de réception et rebroadcast
    while (1)
    {
        ssize_t n = recv(iencli->sock, chaine, sizeof(chaine) - 1, 0);
        last = iencli->sock;
        if (n > 0)
        {
            chaine[n] = '\0';
            printf("[%s] %s", iencli->nickname, chaine);
            write(tube[1], chaine, n);
        }
        else
        {
            printf("Client %s déconnecté.\n", iencli->nickname);
            break;
        }
    }

    close(iencli->sock);
    list_remove_element(list_user, iencli);
    user_free(iencli);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (pipe(tube) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int port = (argc < 2) ? PORT_FREESCORD : atoi(argv[1]);
    int srv = create_listening_sock(port);
    if (srv < 0)
        exit(EXIT_FAILURE);

    list_user = list_create();
    printf("Serveur Freescord lancé sur le port %d...\n", port);

    if (pthread_create(&repeteur, NULL, fonc_thread, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    pthread_detach(repeteur);

    while (1)
    {
        struct user *iencli = user_accept(srv);
        if (!iencli || iencli->sock < 0)
        {
            user_free(iencli);
            continue;
        }

        // Envoi du message de bienvenue (texte + ligne vide)
        char message[] =
            "Bienvenue sur Freescord !\n"
            "Veuillez choisir un pseudonyme qui commence par nickname(max 16, pas de ':')\n"
            "\n";
        lf_to_crlf(message);
        send(iencli->sock, message, strlen(message), 0);

        // Lecture et validation du pseudo
        char chaine[512];
        ssize_t n = recv(iencli->sock, chaine, sizeof(chaine) - 1, 0);
        if (n <= 0)
        {
            close(iencli->sock);
            user_free(iencli);
            continue;
        }
        chaine[strlen(chaine)] = '\0';

        const char *prefix = "nickname ";
        if (strncmp(chaine, prefix, strlen(prefix)) != 0)
        {
            char rep[] = "3 Commande invalide\n";
            lf_to_crlf(rep);
            send(iencli->sock, rep, strlen(rep), 0);
            close(iencli->sock);
            user_free(iencli);
            continue;
        }

        // Extraction du pseudo (sans CRLF ni LF)
        char *pseudo = chaine + strlen(prefix);
        crlf_to_lf(pseudo);

        // Validation du pseudo
        size_t len = strlen(pseudo);
        if (len == 0 || len > 16)
        {
            char rep[] = "1 Taille du Pseudonyme invalide\n";
            lf_to_crlf(rep);
            send(iencli->sock, rep, strlen(rep), 0);
            close(iencli->sock);
            user_free(iencli);
            continue;
        }
        if (strchr(pseudo, ':'))
        {
            char rep[] = "2 Caractère interdit\n";
            lf_to_crlf(rep);
            send(iencli->sock, rep, strlen(rep), 0);
            close(iencli->sock);
            user_free(iencli);
            continue;
        }

        // OK : on stocke et on répond 0
        strncpy(iencli->nickname, pseudo, sizeof(iencli->nickname) - 1);
        iencli->nickname[strlen(iencli->nickname) - 1] = '\0';
        char rep0[] = "0 Bienvenue !\n";
        lf_to_crlf(rep0);
        send(iencli->sock, rep0, strlen(rep0), 0);

        // Lancement du thread de réception
        pthread_t th;
        if (pthread_create(&th, NULL, handle_client, iencli) != 0)
        {
            perror("pthread_create");
            close(iencli->sock);
            user_free(iencli);
            continue;
        }
        pthread_detach(th);
    }

    close(srv);
    return 0;
}
