#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "hash.h"

int write_(char **fd, char *buf, int size, Hash D);

int read_(FILE *fd, char *buf, char diff)
{
	int d, c;

	d = 0;
	c = -1;
	while ((c = fread(buf + d, sizeof(char), 1, fd)) > 0) {
		d += c;
		if (*(buf + d - 1) == diff) {
			if (d - 2 >= 0) {
				if (*(buf + d - 2) == '\\')
					d -= 2;
				else
					break;
			}
		}
	}
	if (c == 0)
		return d * (-1);
	else
		return d;
}


char *xstrtok(char *buf, char *delim, int *err)
{
	int size_buf, size_delim, change, start, s, i, j, ant, wr;
	char *aux;

	size_buf = strlen(buf);
	size_delim = strlen(delim);
	change = 0;
	aux = (char *) calloc(256, sizeof(char));
	if (!aux) {
		*err = 12;
		return NULL;
	}
	start = 0;
	wr = 0;
	while (buf[start] == ' ' || buf[start] == '=')
		start++;
	for (i = start, s = 0; i < size_buf; i++) {
		change = 0;
		for (j = 0; j < size_delim; j++) {
			if (buf[i] == delim[j]) {
				change = 1;
				break;
			}
		}
		if (change == 1)
			break;
		if (buf[i] == ' ' && ant == 0) {
			ant = 1;
			memcpy(aux + s++, buf + i, 1);
		} else if (buf[i] != ' ') {
			ant = 0;
			memcpy(aux + s++, buf + i, 1);
			if (buf[i] == '"')
				wr = 1;
		} else if (wr == 1) {
			memcpy(aux + s++, buf + i, 1);
		}
	}
	memcpy(aux + s, "\0", 1);
	return aux;
}

int parse(char *buf, HashM D)
{
	char *key, *value, *word;
	int bool, err;

	err = 0;
	key = xstrtok(buf, " =\n\t", &err);
	if (err != 0)
		return 12;
	word = xstrtok(buf + strlen(key), "\n", &err);
	if (err != 0)
		return 12;
	if (write_(&value, word, strlen(word), *D) != 0)
		return 12;

	free(word);
	if (!strlen(value)) {
		bool = 0;
		memcpy(value, "\"\"\0", 3);
	} else if ((value[0] >= 'A' && value[0] <= 'z' && value[0] != '\''))
		bool = -1;
	else if (value[0] == '"')
		bool = 0;
	else
		bool = 1;

	err = add_Hash(D, key, value, bool);
	free(value);
	free(key);
	return err;
}

char *xstrdup(char **argv, int *index, int i)
{

	if (argv[*index][i] == '\0') {
		*(index) += 1;
		return argv[(*index)];
	} else if (i != 0)
		return argv[(*index)] + i;
	else
		return argv[*index];

}

int params(int index, char **argv, int argc,
	char **output, char **input, TTList I, HashM D)
{

	char *buf;
	int i, in;

	in = 0;
	for (i = index; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'D') {

				if (strlen(argv[i]) == 2) {
					i += 1;
					if (parse(argv[i], D) != 0)
						return 12;
				} else if (parse(argv[i] + 2, D) != 0) {
					return 12;
				}
			} else if (argv[i][1] == 'I') {

				buf = xstrdup(argv, &i, 2);
				if (add_List(I, buf) != 0)
					return 12;

			} else if (argv[i][1] == 'o') {
				(*output) = xstrdup(argv, &i, 2);
			}
		} else if (i == argc - 1 && in != 0) {
			(*output) = xstrdup(argv, &i, 0);
		} else {
			(*input) = xstrdup(argv, &i, 0);
			in += 1;
		}
	}

	return 0;

}

char *create_path(char *path, char *file, int *err)
{
	char *buf;

	buf = (char *) calloc(strlen(path) + strlen(file) + 1, sizeof(char));
	if (!buf) {
		*err = 12;
		return NULL;
	}
	memcpy(buf, path, strlen(path));
	memcpy(buf + strlen(path), file, strlen(file) + 1);
	return buf;
}

FILE *xopen(char *file, char *path, Tlist I, char *flags, int *err)
{
	char *aux, *fd, *buf;
	FILE *fo;

	aux = (char *) calloc(strlen(file) + 1, sizeof(char));
	if (!aux) {
		*err = 12;
		return NULL;
	}
	memcpy(aux, file, strlen(file) + 1);
	fd = strtok(aux, " \"\n");
	fo = fopen(fd, flags);

	if (fo) {
		free(aux);
		return fo;
	}
	buf = create_path(path, fd, err);
	if (*err != 0) {
		free(aux);
		return NULL;
	}
	fo = fopen(buf, flags);
	if (fo) {
		free(aux);
		free(buf);
		return fo;
	}
	free(buf);
	while (I) {
		buf = create_path(I->path, fd, err);
		if (*err != 0) {
			free(aux);
			return NULL;
		}
		fo = fopen(buf, flags);
		if (fo) {
			free(aux);
			free(buf);
			return fo;
		}
		free(buf);
		I = I->urm;
	}
	*err = 5;
	free(aux);
	return NULL;
}
int xstrncmp(char *buf, char *find, int len, int *index)
{
	int buf_len, i, j, flag;

	buf_len = strlen(buf);
	flag = 0;
	for (i = 0; i < buf_len - len + 1; i++) {
		if (strncmp(buf + i, find, len) == 0 && flag == 0) {
			for (j = i + len; ; j++) {
				if (buf[j] != '\t' && buf[j] != ' ') {
					*index = j;
					return j;
				}
			}
		}
		if (buf[i] == '"' && flag == 0)
			flag = 1;
		else if (buf[i] == '"')
			flag = 0;
	}
	return -1;
}
int validation(char c)
{
	if (c >= 'A' && c <= 'Z')
		return 1;
	else if (c >= 'a' && c <= 'z')
		return 1;
	else
		return 0;

}
int write_(char **fd, char *buf, int size, Hash D)
{
	char *value, *key, *buffer;
	int wr, i, dim, err;

	wr = 1;
	dim = 0;
	err = 0;
	buffer = (char *) calloc(256, sizeof(char));
	if (!buffer)
		return 12;
	for (i = 0; i < size; i++) {
		value = get_Hash(D, buf + i, &value, &key, &err);
		if (value && wr == 1 && validation(buf[i]) == 1) {
			i += strlen(key) - 1;
			memcpy(buffer + dim, value, strlen(value));
			dim += strlen(value);
		} else {
			if (buf[i] == '"' && wr == 0)
				wr = 1;
			else if (buf[i] == '"')
				wr = 0;
			memcpy(buffer + (dim++), buf + i, 1);
		}
		if (err == 12)
			return 12;
	}
	memcpy(buffer + (dim++), "\0", 1);
	*fd = buffer;
	return 0;
}

int if_(char *buf, Hash D, int *err)
{
	char *aux, *eq;
	int i, num;

	if (write_(&aux, buf, strlen(buf), D) != 0) {
		*err = 12;
		return 0;
	}
	eq = strtok(aux, "\n ");
	num  = 0;
	for (i = 0; eq[i] >= '0' && eq[i] <= '9'; i++)
		num = num * 10 + eq[i] - '0';
	free(aux);
	return num != 0;
}


int xxread(FILE *fd, FILE *fo, char *PATH, TTList I, HashM D, int *err)
{
	char *buf, *key, *value;
	int size, wr, index, bool;
	FILE *fi;

	size = 0;
	wr = 1;
	index = 0;
	buf = NULL;
	fi = NULL;
	buf = (char *) calloc(256, sizeof(char));
	if (!buf)
		return 12;
	bool = 0;
	while ((size = read_(fd, buf, '\n')) > 0) {
		if (wr == 1) {
			if (xstrncmp(buf, "#include", 8, &index) >= 0) {
				if (buf[index] != '<') {
					fi =  xopen(buf + index, PATH,
							*I, "r", err);
					if (!fi)
						return *err;

					if (xxread(fi, fo, PATH,
						I, D, err) != 0)
						return *err;
					fclose(fi);
				} else {
					fwrite(buf, sizeof(char), size, fo);
				}

			} else if (xstrncmp(buf, "#ifdef", 6, &index) >= 0) {
				value = get_Hash(*D, buf + index,
						&key, &value, err);
				if (*err == 12)
					return *err;
				if (value == NULL)
					wr = 0;
				else
					wr = 1;

			} else if (xstrncmp(buf, "#ifndef", 7, &index) >= 0) {
				value = get_Hash(*D, buf + index,
						&key, &value, err);
				if (*err == 12)
					return *err;
				if (value == NULL)
					wr = 1;
				else
					wr = 0;

			} else if (xstrncmp(buf, "#if", 3, &index) >= 0) {
				wr = if_(buf + index, *D, err);
				if (*err != 0)
					return 12;
			} else if (xstrncmp(buf, "#define", 7, &index) >= 0) {
				if (parse(buf + index, D) != 0)
					return 12;
			} else if (xstrncmp(buf, "#undef", 6, &index) >= 0) {
				if (remove_Hash(D, buf + index) != 0)
					return 12;
			} else if (xstrncmp(buf, "#else", 5, &index) >= 0)
				wr = 0;
			else if (xstrncmp(buf, "#endif", 6, &index) < 0) {
				if (write_(&value, buf, size, *D) != 0)
					return 12;
				fwrite(value, sizeof(char), strlen(value), fo);
				free(value);
			}

		} else {

			if (xstrncmp(buf, "#elif", 5, &index) >= 0) {
				wr = if_(buf + index, *D, err);
				if (*err != 0)
					return *err;
			} else if (xstrncmp(buf, "#else", 5, &index) >= 0) {
				wr = 1;
			} else if (xstrncmp(buf, "#endif", 6, &index) >= 0) {
				wr = 1;
			}
		}
		free(buf);
		buf = (char *) calloc(256, sizeof(char));
		if (!buf)
			return 12;

	}
	fwrite(buf, sizeof(char), -1 * size, fo);
	free(buf);
	return 0;
}


char *path(char *input, int *err)
{
	char *buf;
	int i;

	for (i = strlen(input); input[i] != '/' && i != 0; i--)
		;

	if (input[i] != '/')
		i -= 1;

	buf = (char *) calloc(i + 4, sizeof(char));
	if (!buf) {
		*err = 12;
		return NULL;
	}
	memcpy(buf, input, i + 1);
	memcpy(buf + i + 2, "\0", 1);
	return buf;
}

int main(int argc, char **argv)
{
	FILE *fi, *fo;
	Hash D;
	Tlist I;
	char *output, *input, *PATH;
	int err;

	fi = NULL;
	fo = NULL;
	D = NULL;
	I = NULL;
	output = NULL;
	input = NULL;
	PATH = NULL;
	err = 0;

	if (params(1, argv, argc, &output, &input, &I, &D) != 0)
		return 12;

	if (input != NULL) {
		PATH = path(input, &err);
		if (err != 0)
			return err;
		fi = fopen(input, "r");
		if (!fi)
			return 5;
	} else {
		fi = stdin;
	}

	if (output != NULL) {
		fo = fopen(output, "w");
		if (!fo)
			return 12;
	} else {
		fo = stdout;
	}

	err = xxread(fi, fo, PATH, &I, &D, &err);

	if (err != 0)
		return err;
	if (PATH != NULL)
		free(PATH);
	if (fi != stdin)
		fclose(fi);
	if (fo != stdout)
		fclose(fo);
	free_hash(D);
	free_list(I);
	return 0;
}
