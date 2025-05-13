/*Jean-Samuel PIERRE 12201116
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain.*/
#include "../include/utils.h"
#include <string.h>
#include <stdlib.h>

char *crlf_to_lf(char *line_with_crlf)
{
	if (!line_with_crlf)
	{
		return NULL;
	}

	size_t len = strlen(line_with_crlf);
	size_t write_pos = 0;

	for (size_t read_pos = 0; read_pos < len; read_pos++)
	{
		if (line_with_crlf[read_pos] == '\r' && line_with_crlf[read_pos + 1] == '\n')
		{
			read_pos++;
		}

		line_with_crlf[write_pos++] = line_with_crlf[read_pos];
	}

	line_with_crlf[write_pos] = '\0';

	return line_with_crlf;
}

char *lf_to_crlf(char *line_with_lf)
{
	if (!line_with_lf)
	{
		return NULL;
	}

	size_t len = strlen(line_with_lf);

	size_t lf_count = 0;
	for (size_t i = 0; i < len; i++)
	{
		if (line_with_lf[i] == '\n')
		{
			lf_count++;
		}
	}

	if (lf_count == 0)
	{
		return line_with_lf;
	}

	size_t write_pos = len + lf_count;
	size_t read_pos = len;

	line_with_lf[write_pos] = '\0';

	while (read_pos > 0)
	{
		read_pos--;
		write_pos--;

		if (line_with_lf[read_pos] == '\n')
		{
			line_with_lf[write_pos--] = '\n';
			line_with_lf[write_pos] = '\r';
		}
		else
		{
			line_with_lf[write_pos] = line_with_lf[read_pos];
		}
	}

	return line_with_lf;
}