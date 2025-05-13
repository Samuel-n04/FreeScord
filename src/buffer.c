

/*Jean-Samuel PIERRE 12201116
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain.*/

#include "../include/buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct buffer
{
	int fd;		   // Descripteur de fichier
	char *chaine;  // Buffer
	size_t size;   // Taille du buffer
	size_t indice; // Indice ou on se trouve dans le buffer
	size_t used;   // Nombre de caractere dans le buffer
	int fin;	   // Flag to indicate end of file
	int unget;	   // Le caractère a remettre dans le buffer
	int active;	   // Le flag pour savoir si y a un caractère a remettre
};

buffer *buff_create(int fd, size_t buffsz)
{
	if (fd < 0 || buffsz == 0)
	{
		return NULL;
	}

	buffer *b = malloc(sizeof(buffer));
	if (!b)
	{
		return NULL;
	}

	// Allocate buffer chaine
	b->chaine = malloc(buffsz * sizeof(char));
	if (!b->chaine)
	{
		free(b);
		return NULL;
	}

	// Initialize buffer fields
	b->fd = fd;
	b->size = buffsz;
	b->indice = 0;
	b->used = 0;
	b->fin = 0;
	b->unget = 0;
	b->active = 0;

	return b;
}

int buff_getc(buffer *b)
{
	// Check for unget character first
	if (b->active)
	{
		b->active = 0;
		return b->unget;
	}

	// If buffer is empty, try to fill it
	if (b->indice >= b->used)
	{
		// Reset buffer
		b->indice = 0;
		b->used = 0;

		// Read into buffer
		ssize_t bytes_read = read(b->fd, b->chaine, b->size);

		if (bytes_read < 0)
		{
			// Read error
			return EOF;
		}

		if (bytes_read == 0)
		{
			// End of file
			b->fin = 1;
			return EOF;
		}

		b->used = bytes_read;
	}

	// Return next character
	return (unsigned char)b->chaine[b->indice++];
}

int buff_ungetc(buffer *b, int c)
{
	// Only one unget is guaranteed
	if (b->active)
	{
		return EOF;
	}

	b->unget = c;
	b->active = 1;
	return c;
}

void buff_free(buffer *b)
{
	if (b)
	{
		free(b->chaine);
		free(b);
	}
}

int buff_eof(const buffer *b)
{
	// End of file if no more characters in buffer and EOF reached
	return b ? (b->fin && b->indice >= b->used) : 1;
}

int buff_ready(const buffer *b)
{
	// Characters are ready if unget is active or buffer has unread characters
	return b ? (b->active || b->indice < b->used) : 0;
}

char *buff_fgets(buffer *b, char *dest, size_t size)
{
	if (!b || !dest || size == 0)
	{
		return NULL;
	}

	size_t i = 0;
	int c;

	// Read up to size-1 characters or until newline
	while (i < size - 1)
	{
		c = buff_getc(b);

		if (c == EOF)
		{
			// EOF or error
			if (i == 0)
			{
				return NULL;
			}
			break;
		}

		dest[i++] = c;

		// Stop if newline
		if (c == '\n')
		{
			break;
		}
	}

	// Null-terminate the string
	dest[i] = '\0';

	return dest;
}

char *buff_fgets_crlf(buffer *b, char *dest, size_t size)
{
	if (!b || !dest || size == 0)
	{
		return NULL;
	}

	size_t i = 0;
	int c;
	int last_was_cr = 0;

	// Read up to size-1 characters or until \r\n
	while (i < size - 1)
	{
		c = buff_getc(b);

		if (c == EOF)
		{
			// EOF or error
			if (i == 0)
			{
				return NULL;
			}
			break;
		}

		dest[i++] = c;

		// Check for \r\n sequence
		if (last_was_cr && c == '\n')
		{
			break;
		}

		// Track if last character was \r
		last_was_cr = (c == '\r');
	}

	// Null-terminate the string
	dest[i] = '\0';

	return dest;
}