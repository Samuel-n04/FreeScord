/*Jean-Samuel PIERRE 12201116
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain.*/

#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../include/buffer.h"
#include "../include/utils.h"

#define MAX 100
#define PORT_FREESCORD 4321
#define ADDR_LOCALE "127.0.0.1"

int connect_serveur_tcp(char *adresse, uint16_t port);

int main(int argc, char *argv[])
{
	char buf_envoi[MAX];
	char buf_recep[MAX];

	int port = argc < 2 ? PORT_FREESCORD : atoi(argv[1]);
	int client = connect_serveur_tcp(ADDR_LOCALE, port);
	if (client < 0)
		exit(1);

	while (1)
	{
		printf("Entrez un message : ");
		if (fgets(buf_envoi, MAX, stdin) == NULL)
			break;

		send(client, buf_envoi, strlen(buf_envoi), 0);

		ssize_t r = recv(client, buf_recep, sizeof(buf_recep) - 1, 0);
		if (r <= 0)
			break;
		buf_recep[r] = '\0';
		printf("Réponse du serveur : %s", buf_recep);
	}

	close(client);
	return 0;
}

int connect_serveur_tcp(char *adresse, uint16_t port)
{
	int socket_client = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_client < 0)
	{
		perror("Echec creation socket");
		return -1;
	}

	struct sockaddr_in addClient;
	addClient.sin_family = AF_INET;
	addClient.sin_port = htons(port);

	if (inet_pton(AF_INET, adresse, &addClient.sin_addr) <= 0)
	{
		perror("Echec de la conversion en binaire");
		close(socket_client);
		return -1;
	}

	if (connect(socket_client, (struct sockaddr *)&addClient, sizeof(addClient)) < 0)
	{
		perror("Echec de la connexion");
		close(socket_client);
		return -1;
	}

	return socket_client;
}
