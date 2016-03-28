#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "skiplist.h"

#define MAX_BUFFER 100

void usage(const char *command) {
	printf("usage : %s -id num\n", command);
	printf("where id is :\n");
	printf("\tc : construct and print the skiplist with data read from file test_files/construct_num.txt\n");
	printf("\ts : construct the skiplist with data read from file test_files/construct_num.txt and search elements from file test_files/search_num..txt\n\t\tPrint statistics about the searches.\n");
	printf("\ti : construct the skiplist with data read from file test_files/construct_num.txt and search, using an iterator, elements read from file test_files/search_num.txt\n\t\tPrint statistics about the searches.\n");
	printf("\tr : construct the skiplist with data read from file test_files/construct_num.txt, remove values read from file test_files/remove_num.txt and print the list in reverse order\n");
	printf("where num is the file number for input\n");
}

char* construire_nom (char* prefix, int num) {
	char buffer[MAX_BUFFER];
	int i;
	for (i = 0; i < MAX_BUFFER; i++)
		buffer[i] = '\0';
	i = 0;
	while (num > 0) {
		buffer[i] = num % 10 + 48;
		num /= 10;
		++i;
	}
	char* nom_fichier = (char*)malloc(sizeof(char)*(strlen(prefix)+strlen(buffer)+strlen(".txt")+1));
	strcpy(nom_fichier, prefix);
	strcat(nom_fichier, buffer);
	strcat(nom_fichier, ".txt");
	return nom_fichier;
}

SkipList construire_liste(int num) {
	FILE* fichier = NULL;
	char* nom_fichier = construire_nom("test_files/construct_", num);
	char buffer[MAX_BUFFER];
	if ((fichier = fopen(nom_fichier, "r")) == NULL) {
		perror(nom_fichier);
		exit(1);
	}
	free(nom_fichier);
	SkipList sk = skiplist_create(atoi(fgets(buffer, MAX_BUFFER, fichier)));
	int nb_valeur = atoi(fgets(buffer, MAX_BUFFER, fichier));
	int nb;
	for (int i = 0; i < nb_valeur; i++) {
		nb = atoi(fgets(buffer, MAX_BUFFER, fichier));
		skiplist_insert(sk, nb);
	}
	fclose(fichier);
	return sk;
}

void test_construction(int num) {
	SkipList sk = construire_liste(num);
	printf("Skiplist (%d)\n", skiplist_size(sk));
	for (unsigned int i = 0; i < skiplist_size(sk); i++) {
		int ith = skiplist_ith(sk, i);
		printf("%d ", ith);
	}
    skiplist_delete(sk);
}

void afficher_stat(SkipList sk, unsigned int nb_valeur, unsigned int nb_found, unsigned int min, unsigned int max, unsigned int total_operations) {
	printf("Statistics : \n");
	printf("    Size of the list : %d\n", skiplist_size(sk));
	printf("Search %d values :\n", nb_valeur);
	printf("    Found %d\n", nb_found);
	printf("    Not found %d\n", nb_valeur - nb_found);
	printf("    Min number of operations : %d\n", min);
	printf("    Max number of operations : %d\n", max);
	printf("    Mean number of operations : %d\n", total_operations / nb_valeur);
}

void test_search(int num) {
	SkipList sk = construire_liste(num);
	char* nom_fichier = construire_nom("test_files/search_", num);
	FILE* fichier = NULL;
	if ((fichier = fopen(nom_fichier, "r")) == NULL) {
		perror(nom_fichier);
		exit(1);
	}
	free(nom_fichier);
	char buffer[MAX_BUFFER];
	unsigned int nb_valeur = (unsigned int)atoi(fgets(buffer, MAX_BUFFER, fichier));
	unsigned int nb_found = 0;
	unsigned int min = skiplist_size(sk);
	unsigned int max = 0;
	unsigned int nb_operations = 0;
	unsigned int total_operations = 0;
	for (unsigned int i = 0; i < nb_valeur; i++) {
		int nb = atoi(fgets(buffer, MAX_BUFFER, fichier));
		if (skiplist_search(sk, nb, &nb_operations)) {
			printf("%d -> true\n", nb);
			nb_found++;
		} else
			printf("%d -> false\n", nb);
		total_operations += nb_operations;
		if (min > nb_operations)
			min = nb_operations;
		if (max < nb_operations)
			max = nb_operations;
	}
	fclose(fichier);
	afficher_stat(sk, nb_valeur, nb_found, min, max, total_operations);
	skiplist_delete(sk);
}

void test_search_iterator(int num){
	SkipList sk = construire_liste(num);
	char* nom_fichier = construire_nom("test_files/search_", num);
	FILE* fichier = NULL;
	if ((fichier = fopen(nom_fichier, "r")) == NULL) {
		perror(nom_fichier);
		exit(1);
	}
	free(nom_fichier);
	char buffer[MAX_BUFFER];
	unsigned int nb_valeur = (unsigned int)atoi(fgets(buffer, MAX_BUFFER, fichier));
	unsigned int nb_found = 0;
	unsigned int min = skiplist_size(sk);
	unsigned int max = 0;
	unsigned int total_operations = 0;
	bool trouve;
	SkipListIterator it = skiplist_iterator_create(sk, FORWARD_ITERATOR);
	for (unsigned int i = 0; i < nb_valeur; i++) {
		int nb = atoi(fgets(buffer, MAX_BUFFER, fichier));
		unsigned int nb_operations = 0;
		trouve = false;
		for (it = skiplist_iterator_begin(it); !skiplist_iterator_end(it); it = skiplist_iterator_next(it)) {
			nb_operations++;
			if (skiplist_iterator_value(it) == nb) {
				trouve = true;
				nb_found++;
				break;
			}
		}
		if (trouve)
			printf("%d -> true\n", nb);
		else
			printf("%d -> false\n", nb);

		if (min > nb_operations)
			min = nb_operations;
		if (max < nb_operations)
			max = nb_operations;
		total_operations += nb_operations;
	}
	afficher_stat(sk, nb_valeur, nb_found, min, max, total_operations);
	fclose(fichier);
	skiplist_delete(sk);
	skiplist_iterator_delete(it);
}

void test_remove(int num){
	SkipList sk = construire_liste(num);
	char* nom_fichier = construire_nom("test_files/remove_", num);
	FILE* fichier = NULL;
	if ((fichier = fopen(nom_fichier, "r")) == NULL) {
		perror(nom_fichier);
		exit(1);
	}
	free(nom_fichier);
	char buffer[MAX_BUFFER];
	unsigned int nb_valeur = (unsigned int)atoi(fgets(buffer, MAX_BUFFER, fichier));
	for (unsigned int i = 0; i < nb_valeur; i++) {
		int nb = atoi(fgets(buffer, MAX_BUFFER, fichier));
		skiplist_remove(sk, nb);
	}
	fclose(fichier);
	printf("Skiplist (%d)\n", skiplist_size(sk));
	SkipListIterator it = skiplist_iterator_create(sk, BACKWARD_ITERATOR);
	for (it = skiplist_iterator_begin(it); !skiplist_iterator_end(it); it = skiplist_iterator_next(it))
		printf("%d ", skiplist_iterator_value(it));
	skiplist_iterator_delete(it);
	skiplist_delete(sk);
}

void generate(int nbvalues);


int main(int argc, const char *argv[]){
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}
	switch (argv[1][1]) {
		case 'c' :
			test_construction(atoi(argv[2]));
			break;
		case 's' :
			test_search(atoi(argv[2]));
			break;
		case 'i' :
			test_search_iterator(atoi(argv[2]));
			break;
		case 'r' :
			test_remove(atoi(argv[2]));
			break;
		case 'g' :
			generate(atoi(argv[2]));
			break;
		default :
			usage(argv[0]);
			return 1;
	}
	return 0;
}

void generate(int nbvalues){
	FILE *output;
	int depth;
	int maxvalue;
	output = fopen("construct.txt", "w");
	srand(nbvalues);
	depth = rand()%16;
	maxvalue = rand()%10 * nbvalues;
	fprintf(output, "%d\n%d\n", depth, nbvalues);
	for (int i=0; i< nbvalues; ++i) {
		fprintf(output, "%d\n", rand()%maxvalue);
	}
	fclose(output);
	output = fopen("search.txt", "w");
	srand(rand());
	nbvalues *= depth/4;
	fprintf(output, "%d\n", nbvalues);
	for (int i=0; i< nbvalues; ++i) {
		fprintf(output, "%d\n", rand()%maxvalue);
	}
	fclose(output);
}


