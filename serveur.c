/* Jean-Samuel PIERRE 12201116
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain. */

#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list/list.h"
#include "user.h"

#define PORT_FREESCORD 4321

#define SOCK_ERR(x, msg)                                                      \
	if ((x) < 0)                                                              \
	{                                                                         \
		fprintf(stderr, "[SERVER ERROR] - %s: %s\n", (msg), strerror(errno)); \
		exit(1);                                                              \
	}

// Création de la socket d'écoute
int create_listening_sock(uint16_t port)
{
	int socket_serv = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_serv < 0)
	{
		perror("Échec création socket serveur");
		return -1;
	}

	struct sockaddr_in addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_addr.s_addr = INADDR_ANY;
	addrServ.sin_port = htons(port);

	if (bind(socket_serv, (struct sockaddr *)&addrServ, sizeof(addrServ)) < 0)
	{
		perror("Échec du bind");
		close(socket_serv);
		return -1;
	}

	if (listen(socket_serv, 10) < 0)
	{
		perror("Échec du listen");
		close(socket_serv);
		return -1;
	}

	return socket_serv;
}

// Thread de gestion client
void *handle_client(void *clt)
{
	struct user *iencli = (struct user *)clt;
	char buff[100];

	while (1)
	{
		ssize_t size = recv(iencli->sock, buff, sizeof(buff) - 1, 0);
		if (size > 0)
		{
			buff[size] = '\0';
			printf("Message reçu : %s\n", buff);
			send(iencli->sock, buff, size, 0);
		}
		else if (size == 0)
		{
			// Client a fermé la connexion proprement
			printf("Client déconnecté.\n");
			break;
		}
		else
		{
			perror("Erreur réception");
			break;
		}
	}

	close(iencli->sock);
	user_free(iencli);
	return NULL;
}

int main(int argc, char *argv[])
{
	int port = argc < 2 ? PORT_FREESCORD : atoi(argv[1]);
	int socket_serv = create_listening_sock(port);
	if (socket_serv < 0)
		exit(1);

	printf("Serveur Freescord lancé sur le port %d...\n", port);

	while (1)
	{
		struct user *iencli = user_accept(socket_serv);
		if (iencli == NULL || iencli->sock < 0)
		{
			perror("Échec accept");
			user_free(iencli);
			continue;
		}

		pthread_t t1;
		if (pthread_create(&t1, NULL, handle_client, iencli) != 0)
		{
			perror("Erreur création thread");
			close(iencli->sock);
			user_free(iencli);
			continue;
		}
		pthread_detach(t1); // Pas besoin de join
	}

	close(socket_serv);
	return 0;
}
