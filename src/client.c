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
#define MAX 512
#define PORT_FREESCORD 4321
#define ADDR_LOCALE "127.0.0.1"

int connect_serveur_tcp(char *adresse, uint16_t port);

int main(int argc, char *argv[])
{
	char buf_envoi[MAX];
	char buf_recep[MAX];
	int port = argc < 2 ? PORT_FREESCORD : atoi(argv[1]);
	int client = connect_serveur_tcp(ADDR_LOCALE, port);

	// Création du buffer pour la lecture bufferisée
	buffer *recep = buff_create(client, MAX);
	if (recep == NULL)
	{
		perror("Échec création buffer");
		close(client);
		exit(1);
	}

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
		// Vérifier d'abord s'il reste des données à lire dans le buffer
		if (buff_ready(recep))
		{
			// Il y a déjà des données dans le buffer, traiter immédiatement
			if (buff_fgets_crlf(recep, buf_recep, MAX) != NULL)
			{
				// Conversion CRLF vers LF pour l'affichage
				crlf_to_lf(buf_recep);
				printf("Message de mon interlocuteur : %s", buf_recep);
				continue; // Continuer pour traiter d'éventuelles autres lignes complètes
			}
		}

		// Attendre uniquement s'il n'y a pas de données disponibles dans le buffer
		int monPoll = poll(tab, 2, -1);
		if (monPoll < 0)
		{
			perror("Echec dans le POLL");
			exit(1);
		}

		// Gestion de l'entrée standard (envoi de messages)
		if (tab[0].revents & (POLLIN | POLLHUP))
		{
			if (fgets(buf_envoi, MAX, stdin) == NULL)
				break;

			// Convertir les LF en CRLF avant d'envoyer
			char *message_crlf = lf_to_crlf(buf_envoi);
			size_t size = strlen(message_crlf);

			// Écriture complète du message dans le socket (intégré dans le main)
			size_t ecrit = 0;
			while (ecrit < size)
			{
				ssize_t res = write(client, message_crlf + ecrit, size - ecrit);

				if (res < 0)
				{
					// Erreur d'écriture
					if (errno == EINTR)
					{
						// Signal interrompu, réessayer
						continue;
					}
					perror("Erreur lors de l'envoi du message");
					close(client);
					buff_free(recep);
					exit(1);
				}
				else if (res == 0)
				{
					// Connexion fermée
					printf("Connexion fermée pendant l'envoi\n");
					close(client);
					buff_free(recep);
					exit(1);
				}

				ecrit += res;
			}

			printf("Message envoye : %s", buf_envoi);
		}

		// Gestion des données reçues depuis le serveur
		if (tab[1].revents & (POLLIN | POLLHUP))
		{
			// Tenter de lire une ligne complète du buffer
			if (buff_fgets_crlf(recep, buf_recep, MAX) == NULL)
			{
				// Erreur ou fin de fichier
				if (buff_eof(recep))
				{
					printf("Connexion fermée par le serveur\n");
				}
				else
				{
					perror("Erreur lors de la lecture");
				}
				break;
			}

			// Conversion de CRLF vers LF pour l'affichage local
			crlf_to_lf(buf_recep);
			printf("Message de mon interlocuteur : %s", buf_recep);
		}
	}

	// Libération des ressources
	buff_free(recep);
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