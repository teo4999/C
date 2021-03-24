/**
 * Hossu Teodor-Ioan
 * 315CB
 */

#ifndef _TRIE_H_
#define _TRIE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Structura ce va memora caracterul curent din nod, daca acesta este fisier,
 * nodul parinte, urmatorul copil al parintelui si primul copil al nodului
 * curent.
 */
typedef struct node {
	char path;
	int isFile;
	struct node *parent;
	struct node *next;
	struct node *children;
} Node;

// Structura ce va memora radacina arborelui si directorul curent
typedef struct {
	Node *root;
	Node *current;
} Trie;

#define CMDSIZE 6
#define BUFLEN 1000

// Creez un nod nou al arborelui
Node *newNode(char path, int isFile) {
	Node *new = calloc(1, sizeof(Node));
	if (!new) {
		printf("Memory Error!\n");
		exit(EXIT_FAILURE);
	}
	new->path = path;
	new->isFile = isFile;
	return new;
}

// Initializez arborele si radacina acestuia la caracterul '/'
Trie *initTrie() {
	Trie *newTrie = calloc(1, sizeof(Trie));
	if (!newTrie) {
		printf("Memory Error!\n");
		exit(EXIT_FAILURE);
	}
	newTrie->current = newTrie->root = newNode('/', 0);
	return newTrie;
}

// Pornind de la nodul curent, creez noduri in arbore cu restul caii
int completePath(Node *node, char *restOfPath, int type) {
	/** Verific daca mai exista un caracter '/' in afara de cel final, caz in
	 * care nu exista path valid pentru noul fisier
	 */
	char *check = strchr(restOfPath + 1, '/');
	if (check && strchr(check + 1, '/')) {
		return 2;
	}
	// Daca se continua calea
	if (type == 1) {
		if (!strlen(restOfPath)) {
			return 0;
		}
		node->children = newNode(restOfPath[0], 0);
		node->children->parent = node;
		node = node->children;
	} else {
		// Sau daca se creeaza o noua cale pornind de la parintele nodului
		if (!strlen(restOfPath)) {
			return 0;
		}
		node->next = newNode(restOfPath[0], 0);
		node->next->parent = node->parent;
		node = node->next;
	}
	// Se creeaza noduri pentru restul caii
	int i, n = strlen(restOfPath);
	for (i = 1; i < n - 1; i++) {
		node->children = newNode(restOfPath[i], 0);
		node->children->parent = node;
		node = node->children;
	}
	// Se verifica daca calea este director sau fisier
	if (restOfPath[n - 1] == '/') {
		node->children = newNode(restOfPath[n - 1], 0);
	} else {
		node->children = newNode(restOfPath[n - 1], 1);
	}
	node->children->parent = node;
	return 1;
}

int insertNode(Node *node, char charToInsert, char *restOfPath) {
	// Se parcurge calea cat timp aceasta este egala
	if (node->path == charToInsert) {
		/**	Daca mai exista un singur caracter '/' in cale, se adauga ca si
		 * ultim copil
		 */
		if (restOfPath[0] == '/' && strlen(restOfPath) == 1) {
			while (node->next) {
				node = node->next;
			}
			return completePath(node, restOfPath, 2);
		}
		/** Se incearca continuarea parcurgerii pe copilul nodului curent, iar
		 * daca acesta nu are copii, se creeaza restul caii in arbore
		 */
		else if (node->children) {
			return insertNode(node->children, restOfPath[0], restOfPath + 1);
		} else {
			return completePath(node, restOfPath, 1);
		}
	} else {
		/** Se incearca continuarea parcurgerii pe urmatorul copil al nodului
		 * parinte, iar daca acesta nu are copii, se creeaza restul caii in
		 * arbore
		 */
		if (node->next) {
			return insertNode(node->next, charToInsert, restOfPath);
		} else {
			return completePath(node, restOfPath - 1, 2);
		}
	}
}

void insert(Trie *trie, char *path, char *original, int type) {
	// Se incearca inserarea in arbore a caii
	int ok;
	if (type == 1) {
		ok = insertNode(trie->current, path[0], path + 1);
	} else if (type == 2) {
		ok = insertNode(trie->root, path[0], path + 1);
	} else {
		ok = insertNode(trie->current, '/', path);
	}
	// Daca a aparut o eroare pe parcurs, se afiseaza aceasta
	if (ok == 0) {
		fprintf(stderr, "%s: already exists\n", original);
	} else if (ok == 2) {
		fprintf(stderr, "%s: No such file or directory\n", original);
	}
}

Node *searchDir(Node *node, char currentChar, char *restOfPath) {
	// Se parcurge calea cat timp aceasta este egala
	if (node->path == currentChar) {
		if (node->children) {
			// Daca calea nu mai are elemente, atunci se cauta nodul '/'
			if (!strlen(restOfPath)) {
				node = node->children;
				while (node->path != '/') {
					node = node->next;
				}
				return node;
			} else {
				// Altfel se continua parcurgerea
				return searchDir(node->children, restOfPath[0], restOfPath + 1);
			}
		} else {
			// Daca nodul curent nu are copii, atunci calea nu exista
			return NULL;
		}
	} else {
		/** Se incearca continuarea parcurgerii pe urmatorul copil al nodului
		 * parinte, iar daca acesta nu are copii, calea nu exista.
		 */
		if (node->next) {
			return searchDir(node->next, currentChar, restOfPath);
		} else {
			return NULL;
		}
	}
}

Node *goBackDir(Node *current) {
	// Se parcurge arborele pana la urmatorul caracter '/'
	current = current->parent;
	while (current->path != '/') {
		current = current->parent;
	}
	return current;
}

void changeDir(Trie *trie, char *path, char *original, int type) {
	/** Se verifica daca exista caracterele ".." pentru a putea deplasa
	 * directorul curent inapoi. In cazul in care directorul curent se afla
	 * pe radacina arborelui, atunci se afiseaza o eroare
	 */
	int ok = 1;
	while (!strncmp(path, "..", 2)) {
		ok = 0;
		path = path + 2;
		if (trie->current == trie->root) {
			fprintf(stderr, "%s: No such file or directory\n", original);
			return;
		}
		if (path[0] != 0) {
			path++;
		}
		trie->current = goBackDir(trie->current);
	}
	// Daca s-a terminat calea, se iese din functie
	if (path[0] == 0) {
		return;
	}
	// Daca s-a modificat directorul curent, se modifica calea sa inceapa cu '/'
	if (ok == 0) {
		path--;
	}
	/** Se cauta calea pentru a se modifica directorul. Daca nu se gaseste, se
	 * afiseaza o eroare
	 */
	if (type == 1) {
		Node *current = searchDir(trie->current, path[0], path + 1);
		if (!current) {
			fprintf(stderr, "%s: No such file or directory\n", original);
		} else {
			trie->current = current;
		}
	} else if (type == 2) {
		Node *current = searchDir(trie->root, path[0], path + 1);
		if (!current) {
			fprintf(stderr, "%s: No such file or directory\n", original);
		} else {
			trie->current = current;
		}
	} else {
		Node *current = searchDir(trie->current, '/', path);
		if (!current) {
			fprintf(stderr, "%s: No such file or directory\n", original);
		} else {
			trie->current = current;
		}
	}
}

void printFiles(Node *node, char *name, int classify) {
	/** Se parcurge arborele pana la intalnirea unui nod '/', si se afiseaza
	 * un fisier sau un director
	 */
	if (node->path != '/') {
		name[strlen(name)] = node->path;
		if (node->children) {
			printFiles(node->children, name, classify);
		} else {
			if (node->isFile) {
				if (classify == 1) {
					printf("%s* ", name);
				} else {
					printf("%s ", name);
				}
			}
		}
		name[strlen(name) - 1] = 0;
		if (node->next) {
			printFiles(node->next, name, classify);
		}
	} else {
		if (classify == 1) {
			printf("%s/ ", name);
		} else {
			printf("%s ", name);
		}
		if (node->next) {
			printFiles(node->next, name, classify);
		}
	}
}

void list(Trie *trie, int classify, int type) {
	// Se afiseaza toate fisierele din directorul curent
	char buffer[BUFLEN] = {0};
	if (type == 1) {
		if (trie->current->children) {
			printFiles(trie->current->children, buffer, classify);
		}
	}
	printf("\n");
}

char *getDirName(Node *node, char *name) {
	/**	Se parcurge arborele si se returneaza numele inversat al directorului
	 * curent
	 */
	if (!node) {
		return name;
	} else {
		name[strlen(name)] = node->path;
		return getDirName(node->parent, name);
	}
}

char *strrev(char *string) {
	// Se inverseaza un sir de caractere
	int i, n = strlen(string);
	char *rev = calloc(n + 1, sizeof(char));
	if (!rev) {
		printf("Memory Error!\n");
		exit(EXIT_FAILURE);
	}
	for (i = n - 1; i >= 0; i--) {
		rev[n - 1 - i] = string[i];
	}
	return rev;
}

void printDir(Trie *trie) {
	// Afisez numele directorului curent
	char name[BUFLEN] = {0};
	char *dir = getDirName(trie->current, name);
	dir = strrev(dir);
	if (strlen(dir) > 1) {
		dir[strlen(dir) - 1] = 0;
	}
	printf("%s\n", dir);
	free(dir);
}

void deleteNode(Node *node) {
	// Parcurg nodul SDR si il eliberez
	if (node) {
		deleteNode(node->next);

		deleteNode(node->children);

		free(node);
	}
}

void deleteTrie(Trie *trie) {
	// Eliberez toate nodurile si arborele
	deleteNode(trie->root);
	free(trie);
}

#endif
