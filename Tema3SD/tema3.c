/**
 * Hossu Teodor-Ioan
 * 315CB
 */

#include "trie.h"

int main() {
	// Creez un trie ce va memora sistemul de fisiere
	Trie *filesystem = initTrie();

	int n;
	scanf("%d", &n);
	int i;
	char command[CMDSIZE];
	// Parcurg fiecare comanda data de la tastatura
	for (i = 0; i < n; i++) {
		scanf("%s ", command);
		if (!strcmp(command, "mkdir")) {
			// Citesc linia, deoarece se pot crea mai multe directoare
			char buffer[BUFLEN] = {0};
			fgets(buffer, BUFLEN, stdin);
			buffer[strlen(buffer) - 1] = 0;

			/** Parcurg fiecare nume si verific daca este o cale absoluta sau
			 * relativa, apoi apelez functia de inserare in functie de caz.
			 * 1 - cale relativa cu ./
			 * 2 - cale absoluta
			 * 3 - cale relativa de la directorul curent
			 */
			char *path = strtok(buffer, " ");
			while (path) {
				char *dir = calloc(strlen(path) + 2, sizeof(char));
				if (!dir) {
					printf("Memory Error!\n");
					exit(EXIT_FAILURE);
				}
				sprintf(dir, "%s/", path);
				if (path[0] == '.') {
					if (path[1] == '/') {
						insert(filesystem, dir + 1, path, 1);
					} else {
						insert(filesystem, dir, path, 3);
					}
				} else if (path[0] == '/') {
					insert(filesystem, dir, path, 2);
				} else {
					insert(filesystem, dir, path, 3);
				}
				path = strtok(NULL, " ");
				free(dir);
			}
		} else if (!strcmp(command, "cd")) {
			// Modific directorul curent, exista aceleasi cazuri ca la mkdir
			char *path = calloc(BUFLEN, sizeof(char));
			if (!path) {
				printf("Memory Error!\n");
				exit(EXIT_FAILURE);
			}
			scanf("%s", path);
			if (path[0] == '.') {
				if (path[1] == '/') {
					changeDir(filesystem, path + 1, path, 1);
				} else {
					changeDir(filesystem, path, path, 3);
				}
			} else if (path[0] == '/') {
				changeDir(filesystem, path, path, 2);
			} else {
				changeDir(filesystem, path, path, 3);
			}
			free(path);
		} else if (!strcmp(command, "ls")) {
			// Afisez fisierele si directoarele din directorul curent
			list(filesystem, 0, 1);
		} else if (!strcmp(command, "pwd")) {
			// Afisez directorul curent
			printDir(filesystem);
		} else {
			break;
		}
	}

	// Eliberez sistemul de fisiere
	deleteTrie(filesystem);
	return 0;
}
