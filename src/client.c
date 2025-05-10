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
#include <poll.h>
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
	{
		printf("Echec conexion du client");
		exit(1);
	}
	else
	{
		printf("Vous etes connecté au serveur !! \n Envoyez votre premier message \n ");
	}

	struct pollfd tab[2] = {
		{.fd = 0, .events = POLLIN},
		{.fd = client, .events = POLLIN}};

	while (1)

	{

		int monPoll = poll(tab, 2, -1);

		if (monPoll < 0)
		{
			perror("Echec dans le POLL");
			exit(1);
		}

		if (tab[0].revents & (POLLIN | POLLHUP))
		{
			if (fgets(buf_envoi, MAX, stdin) == NULL)
				break;
			// scanf("%s", buf_envoi);
			send(client, buf_envoi, strlen(buf_envoi), 0);
			printf("Message envoye : %s\n", buf_envoi);
		}

		if (tab[1].revents & (POLLIN | POLLHUP))
		{

			ssize_t r = recv(client, buf_recep, sizeof(buf_recep) - 1, 0);
			if (r <= 0)
				break;
			buf_recep[r] = '\0';
			printf("Message de mon interlocuteur : %s", buf_recep);
		}
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
