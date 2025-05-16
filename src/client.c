/* client.c
 * Jean-Samuel PIERRE 12201116
 * Je déclare qu'il s'agit de mon propre travail.
 */

#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/buffer.h"
#include "../include/utils.h"

#define MAX 512
#define PORT_FREESCORD 4321
#define ADDR_LOCALE "127.0.0.1"

int connect_serveur_tcp(const char *adresse, uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in srv = {
		.sin_family = AF_INET,
		.sin_port = htons(port)};
	if (inet_pton(AF_INET, adresse, &srv.sin_addr) <= 0)
	{
		perror("inet_pton");
		close(sock);
		return -1;
	}
	if (connect(sock, (struct sockaddr *)&srv, sizeof(srv)) < 0)
	{
		perror("connect");
		close(sock);
		return -1;
	}
	return sock;
}

int main(int argc, char *argv[])
{
	int port = (argc < 2) ? PORT_FREESCORD : atoi(argv[1]);
	int client = connect_serveur_tcp(ADDR_LOCALE, port);
	if (client < 0)
		exit(EXIT_FAILURE);

	printf("Vous êtes connecté au serveur !!\n");
	buffer *recep = buff_create(client, MAX);
	if (!recep)
	{
		perror("buff_create");
		close(client);
		exit(EXIT_FAILURE);
	}

	char chaine[MAX];

	/* 1) Lecture du message de bienvenue jusqu'à la ligne vide */
	while (buff_fgets_crlf(recep, chaine, MAX))
	{
		crlf_to_lf(chaine);
		/* supprimer '\n' final */
		chaine[strcspn(chaine, "\n")] = '\0';
		if (chaine[0] == '\0')
			break;
		printf("%s\n", chaine);
	}

	/* 2) Envoi du pseudonyme */
	printf("Entrez votre pseudonyme : ");
	if (!fgets(chaine, MAX, stdin))
	{
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	/* supprimer '\n' et ajouter un seul avant conversion */
	chaine[strcspn(chaine, "\n")] = '\0';
	size_t size1 = strlen(chaine);
	if (size1 < MAX - 1)
	{
		chaine[size1] = '\n';
		chaine[size1 + 1] = '\0';
	}
	lf_to_crlf(chaine);
	if (write(client, chaine, strlen(chaine)) < 0)
	{
		perror("write pseudo");
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}

	/* 3) Lecture de la réponse du serveur */
	if (!buff_fgets_crlf(recep, chaine, MAX))
	{
		perror("buff_fgets_crlf");
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	crlf_to_lf(chaine);
	chaine[strcspn(chaine, "\n")] = '\0';
	char code = chaine[0];
	if (code != '0')
	{
		if (code == '1')
			fprintf(stderr, "Erreur : pseudo invalide\n");
		else if (code == '2')
			fprintf(stderr, "Erreur : caractère ':' interdit\n");
		else if (code == '3')
			fprintf(stderr, "Erreur : commande invalide\n");
		else
			fprintf(stderr, "Réponse inattendue : %s\n", chaine);
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	printf("Serveur : %s\n", chaine + 2);

	/* 4) Boucle de communication */
	struct pollfd fds[2] = {
		{.fd = 0, .events = POLLIN},
		{.fd = client, .events = POLLIN}};

	while (1)
	{
		if (buff_ready(recep))
		{
			if (buff_fgets_crlf(recep, chaine, MAX))
			{
				crlf_to_lf(chaine);
				chaine[strcspn(chaine, "\n")] = '\0';
				printf("Reçu : %s\n", chaine);
				continue;
			}
		}
		if (poll(fds, 2, -1) < 0)
			break;

		if (fds[0].revents & POLLIN)
		{
			if (!fgets(chaine, MAX, stdin))
				break;
			chaine[strcspn(chaine, "\n")] = '\0';
			size_t size2 = strlen(chaine);
			if (size2 < MAX - 1)
			{
				chaine[size2] = '\n';
				chaine[size2 + 1] = '\0';
			}
			lf_to_crlf(chaine);
			write(client, chaine, strlen(chaine));
		}

		/* réception serveur */
		if (fds[1].revents & POLLIN)
		{
			if (!buff_fgets_crlf(recep, chaine, MAX))
				break;
			crlf_to_lf(chaine);
			chaine[strcspn(chaine, "\n")] = '\0';
			printf("Message de votre interlocuteur : %s\n", chaine);
		}
	}

	buff_free(recep);
	close(client);
	return 0;
}
