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

	char line[MAX];

	/* 1) Lecture du message de bienvenue jusqu'à la ligne vide */
	while (buff_fgets_crlf(recep, line, MAX))
	{
		crlf_to_lf(line);
		/* supprimer '\n' final */
		line[strcspn(line, "\n")] = '\0';
		if (line[0] == '\0')
			break;
		printf("%s\n", line);
	}

	/* 2) Envoi du pseudonyme */
	printf("Entrez votre pseudonyme : ");
	if (!fgets(line, MAX, stdin))
	{
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	/* supprimer '\n' et ajouter un seul avant conversion */
	line[strcspn(line, "\n")] = '\0';
	size_t L = strlen(line);
	if (L < MAX - 1)
	{
		line[L] = '\n';
		line[L + 1] = '\0';
	}
	lf_to_crlf(line);
	if (write(client, line, strlen(line)) < 0)
	{
		perror("write pseudo");
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}

	/* 3) Lecture de la réponse du serveur */
	if (!buff_fgets_crlf(recep, line, MAX))
	{
		perror("buff_fgets_crlf");
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	crlf_to_lf(line);
	line[strcspn(line, "\n")] = '\0';
	char code = line[0];
	if (code != '0')
	{
		if (code == '1')
			fprintf(stderr, "Erreur : pseudo invalide\n");
		else if (code == '2')
			fprintf(stderr, "Erreur : caractère ':' interdit\n");
		else if (code == '3')
			fprintf(stderr, "Erreur : commande invalide\n");
		else
			fprintf(stderr, "Réponse inattendue : %s\n", line);
		buff_free(recep);
		close(client);
		exit(EXIT_FAILURE);
	}
	printf("Serveur : %s\n", line + 2);

	/* 4) Boucle de communication */
	struct pollfd fds[2] = {
		{.fd = 0, .events = POLLIN},
		{.fd = client, .events = POLLIN}};

	while (1)
	{
		if (buff_ready(recep))
		{
			if (buff_fgets_crlf(recep, line, MAX))
			{
				crlf_to_lf(line);
				line[strcspn(line, "\n")] = '\0';
				printf("Reçu : %s\n", line);
				continue;
			}
		}
		if (poll(fds, 2, -1) < 0)
			break;

		/* saisir et envoyer */
		if (fds[0].revents & POLLIN)
		{
			if (!fgets(line, MAX, stdin))
				break;
			line[strcspn(line, "\n")] = '\0';
			size_t l = strlen(line);
			if (l < MAX - 1)
			{
				line[l] = '\n';
				line[l + 1] = '\0';
			}
			lf_to_crlf(line);
			write(client, line, strlen(line));
		}

		/* réception serveur */
		if (fds[1].revents & POLLIN)
		{
			if (!buff_fgets_crlf(recep, line, MAX))
				break;
			crlf_to_lf(line);
			line[strcspn(line, "\n")] = '\0';
			printf("Serveur : %s\n", line);
		}
	}

	buff_free(recep);
	close(client);
	return 0;
}
