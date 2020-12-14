#define DELIMITER "\t[]{}<>=+-*/%!&|^.,:;() .\n"

typedef struct hashmap {
	char *key;
	char *value;
	char *keyP;
	int bool;
	struct hashmap *urm;
} Hashmap, *Hash, **HashM;

void free_hash(Hash H)
{
	Hash p;

	while (H) {
		if (H->key)
			free(H->key);
		if (H->keyP)
			free(H->keyP);
		if (H->value)
			free(H->value);
		p = H;
		H = H->urm;
		free(p);
	}
}


Hash create_Hash(char *key, char *value, char *keyP, int bool)
{
	Hash new;

	new = (Hash) malloc(sizeof(Hashmap));
	if (!new)
		return NULL;
	new->key = (char *) malloc(strlen(key) + 1);
	if (!new->key) {
		free(new);
		return NULL;
	}
	memcpy(new->key, key, strlen(key) + 1);
	new->value = (char *) malloc(strlen(value) + 1);
	if (!new->value) {
		free(new);
		free(new->key);
		return NULL;
	}
	memcpy(new->value, value, strlen(value) + 1);
	new->keyP = (char *) malloc(strlen(keyP) + 1);
	if (!new->keyP) {
		free(new);
		free(new->key);
		free(new->value);
		return NULL;
	}
	memcpy(new->keyP, keyP, strlen(keyP) + 1);
	new->bool = bool;
	new->urm = NULL;
	return new;
}

int change_value(Hash hash,char *key, char *value, int bool)
{
	while (hash) {
		if (strcmp(hash->key, key) == 0) {
			free(hash->value);
			hash->value = (char *) malloc(strlen(value) + 1);
			if (!hash->value)
				return 12;
			memcpy(hash->value, value, strlen(value) + 1);
			hash->bool = bool;
		} else if (strcmp(hash->keyP, key) == 0) {
			if (change_value(hash, hash->key, value, bool) != 0)
				return 12;
		}
		hash = hash->urm;
	}
	return 0;
}

int add_Hash(HashM HashMap, char *key, char *value, int bool)
{
	Hash hash, prev;
	char *keyP;
	int modified, value_overwrite, size;

	hash = *HashMap;
	prev = NULL;
	keyP = malloc(sizeof(char) * (strlen(value) + 1));
	if (!keyP)
		return 12;

	if (bool == -1)
		memcpy(keyP, value, strlen(value) + 1);
	else
		memcpy(keyP, "\0", 1);
	if (hash) {
		modified = 0;
		value_overwrite = 0;
		while (hash) {
			if (strcmp(hash->key, keyP) == 0 && hash->bool != -1) {
				value = hash->value;
				bool = hash->bool;
				value_overwrite = 1;
			}
			hash = hash->urm;
		}
		hash = *HashMap;
		while (hash) {
			if (strcmp(hash->key, key) == 0) {
				if (strcmp(hash->keyP, keyP) != 0) {
					free(hash->keyP);
					hash->keyP = malloc(sizeof(char)
						* strlen(keyP) + 1);
					if (!hash->keyP) {
						free(keyP);
						return 12;
					}
					size = strlen(keyP);
					memcpy(hash->keyP, keyP, size + 1);
				}
				free(hash->value);
				hash->value = malloc(strlen(value) + 1);
				if (!hash->value) {
					free(keyP);
					return 12;
				}
				memcpy(hash->value, value, strlen(value) + 1);
				hash->bool = bool;
				modified = 1;
			} else if (strcmp(hash->keyP, key) == 0) {
				if (change_value(*HashMap, hash->key,
								value, bool)) {
					free(keyP);
					return 12;
				}
			}
			prev = hash;
			hash = hash->urm;
		}
		if (modified == 1)
			return 0;

		prev->urm = create_Hash(key, value, keyP, bool);
		if (!prev->urm) {
			free(keyP);
			return 12;
		}

	} else {
		(*HashMap) = create_Hash(key, value, keyP, bool);
		if (!(*HashMap)) {
			free(keyP);
			return 12;
		}
	}
	free(keyP);
	return 0;
}
int remove_Hash(HashM HashMap, char *KEY)
{
	Hash hash, rm, prev, rmprev;
	char *aux, *key;

	hash = *HashMap;
	rm = NULL;
	prev = NULL;
	rmprev = NULL;
	aux = malloc(sizeof(char) * strlen(KEY) + 1);
	if (!aux)
		return 12;
	memcpy(aux, KEY, strlen(KEY) + 1);
	key = strtok(aux, "\n ");
	while (hash && key) {
		if (strcmp(hash->key, key) == 0) {
			rm = hash;
			rmprev = prev;
		} else if (strcmp(hash->keyP, key) == 0) {
			free(hash->value);
			hash->value = malloc(sizeof(char)
						* strlen(hash->keyP) + 1);
			if (!hash->value) {
				free(aux);
				return 12;
			}
			memcpy(hash->value, hash->keyP, strlen(hash->keyP) + 1);
			hash->bool = -1;
		}
		prev = hash;
		hash = hash->urm;
	}
	if (rm == *HashMap) {
		(*HashMap) = (*HashMap)->urm;
		free(rm->key);
		free(rm->keyP);
		free(rm->value);
		free(rm);
	} else if (rmprev != NULL) {
		rmprev->urm = rm->urm;
		free(rm->key);
		free(rm->keyP);
		free(rm->value);
		free(rm);
	}
	free(aux);
	return 0;
}


char *get_Hash(Hash HashMap, char *key, char **value, char **KEY, int *err)
{
	char *aux, *val;

	aux = malloc(sizeof(char) * strlen(key) + 1);
	if (!aux) {
		*err = 12;
		return NULL;
	}
	memcpy(aux, key, strlen(key) + 1);
	val = strtok(aux, DELIMITER);
	while (HashMap && val) {
		if (strcmp(val, HashMap->key) == 0) {
			*value = HashMap->value;
			*KEY = HashMap->key;
			free(aux);
			return HashMap->value;
		}
		HashMap = HashMap->urm;
	}
	free(aux);
	return NULL;
}
