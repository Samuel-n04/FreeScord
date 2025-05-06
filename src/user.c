
/*Jean-Samuel PIERRE 12201116
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain.*/
#include "../include/user.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

/** accepter une connection TCP depuis la socket d'écoute sl et retourner un
 * pointeur vers un struct user, dynamiquement alloué et convenablement
 * initialisé */
struct user *user_accept(int sl)
{

	struct user *us = malloc(sizeof(struct user));

	if (!us)
	{
		perror("Echec allocation user");
		return NULL;
	}

	us->address = malloc(sizeof(struct sockaddr));

	if (!us->address || us->addr_len < 0)
	{
		perror("Echec alloactiion de l'adresse user");
		return NULL;
	}

	us->addr_len = sizeof(us->address);
	us->sock = accept(sl, us->address, &us->addr_len);
	if (us->sock < 0)
	{
		perror("Échec accept");
		free(us->address);
		free(us);
		return NULL;
	}

	else
	{
		printf("Utilisateur Connecté\n");
	}

	return us;
}

/** libérer toute la mémoire associée à user */
void user_free(struct user *user)
{

	if (user != NULL)
	{
		free(user->address);
		free(user);
		user = NULL;
	}
	/* pour éviter les warnings de variable non utilisée */
}
