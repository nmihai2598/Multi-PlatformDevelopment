typedef struct list {
	char *path;
	struct list *urm;
} List, *Tlist, **TTList;


void free_list(Tlist L)
{
	Tlist p;

	while (L) {
		free(L->path);
		p = L;
		L = L->urm;
		free(p);
	}
}

Tlist create_List(char *path)
{
	Tlist new;

	new = (Tlist) malloc(sizeof(List));
	if (!new)
		return NULL;
	new->path = (char *) malloc((strlen(path) + 2) * sizeof(char));
	if (!new->path) {
		free(new);
		return NULL;
	}
	memcpy(new->path, path, strlen(path) + 1);
	if (new->path[strlen(new->path) - 1] != '/')
		memcpy(new->path + strlen(new->path), "/\0", 2);
	new->urm = NULL;
	return new;
}

int add_List(TTList List, char *path)
{
	Tlist L;

	L = *List;
	if (L) {
		while (L->urm)
			L = L->urm;

		L->urm = create_List(path);
		if (!L->urm) {
			free_list(*List);
			return 12;
		}
	} else {
		(*List) = create_List(path);
		if (!(*List))
			return 12;
	}
	return 0;
}
