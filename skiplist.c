#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rng.h"
#include "skiplist.h"

typedef struct s_node* Noeud;
struct s_node {
    Noeud* suivants;        // Les noeuds suivants
    Noeud* precedents;      // Les noeuds précédents
    unsigned int hauteur;   // La hauteur du noeud
    int valeur;             // La valeur du noeud
};

struct s_SkipList {
    Noeud* premiers;             // Les premiers noeuds de la liste
    Noeud* derniers;             // Les derniers noeuds de la liste
    RNG rngesus;                 // Générateur de nombre aléatoire
    unsigned int hauteur;        // La hauteur maximale de la liste
    unsigned int nb_elements;    // Le nombre de noeuds dans la liste
};

struct s_SkipListIterator {
    SkipList skiplist;
    Noeud noeud;
    bool sens;
};

SkipList skiplist_create(int nb_levels) {
    assert(nb_levels > 0);
    // Alloue en mémoire une liste à raccourci
    SkipList sk = (SkipList)malloc(sizeof(struct s_SkipList));
    assert(sk != NULL);
    // Initialise le générateur de nombre aléatoire
    sk->rngesus = rng_initialize(0);
    // Initialise les tableaux de Noeud
    sk->premiers = (Noeud*)malloc(sizeof(Noeud)*nb_levels);
    assert(sk->premiers != NULL);
    sk->derniers = (Noeud*)malloc(sizeof(Noeud)*nb_levels);
    assert(sk->derniers != NULL);
    for (int i = 0; i < nb_levels; i++) {
        sk->premiers[i] = NULL;
        sk->derniers[i] = NULL;
    }
    // Initialise la hauteur de la liste et le nombre d'éléments
    sk->hauteur = (unsigned int)nb_levels;
    sk->nb_elements = 0;
    return sk;
}

Noeud creer_noeud(SkipList d, int x) {
    // Alloue en mémoire un noeud
    Noeud nd = (Noeud)malloc(sizeof(struct s_node));
    assert(nd != NULL);
    // Génère la hauteur du noeud
    nd->hauteur = (int)rng_get_value(&d->rngesus, d->hauteur-1)+1;
    // Initialise les tableaux de noeuds suivants et précédents
    nd->suivants = (Noeud*)malloc(sizeof(Noeud)*nd->hauteur);
    assert(nd->suivants != NULL);
    nd->precedents = (Noeud*)malloc(sizeof(Noeud)*nd->hauteur);
    assert(nd->precedents != NULL);
    for (int i = 0; i < (int)nd->hauteur; i++) {
        nd->suivants[i] = NULL;
        nd->precedents[i] = NULL;
    }
    // Initialise la valeur du noeud
    nd->valeur = x;
    return nd;
}

/**
 * \brief Détruit un noeud
 * \param nd Noeud à détruire
 */
void detruire_noeud(Noeud nd) {
    free(nd->suivants);
    free(nd->precedents);
    free(nd);
}

void skiplist_delete(SkipList d) {
    // Place le noeud courant sur le premier noeud de la liste
    Noeud courant = d->premiers[0];
    Noeud precedent = NULL;
    while (courant != NULL) {
        // Avance d'un cran le noeud courant
        precedent = courant;
        courant = courant->suivants[0];
        // Détruit le noeud précédent
        detruire_noeud(precedent);
    }
    // Libère en mémoire les tableaux de premiers et derniers noeuds
    free(d->premiers);
    free(d->derniers);
    // Libère en mémoire la skiplist
    free(d);
}

unsigned int skiplist_size(SkipList d) {
    return d->nb_elements;
}

int skiplist_ith(SkipList d, unsigned int i) {
    assert(i < d->nb_elements);
    // Place le noeud courant sur le premier noeud
    Noeud courant = d->premiers[0];
    // Avance le noeud courant jusqu'au ième noeud
    for (unsigned int j = 0; j != i; j++)
        courant = courant->suivants[0];
    return courant->valeur;
}

void skiplist_map(SkipList d, ScanOperator f, void *user_data) {
    Noeud courant = d->premiers[0];
    while (courant != NULL) {
        f(courant->valeur, user_data);
        courant = courant->suivants[0];
    }
}

/**
 * \brief Récupère le premier noeud non null le plus haut d'une liste de noeud
 * et la hauteur à laquelle il a été trouvé
 * \param des_noeuds Une liste de noeud dans laquelle on cherche le premier noeud non null le plus haut
 * \param nd Pointeur sur le noeud à renvoyer
 * \param max La hauteur maximum de la liste de noeud
 * \return La hauteur a laquelle a été trouvé le noeud non null le plus haut
 */
int noeud_haut(Noeud* des_noeuds, Noeud* nd, int max) {
    *nd = des_noeuds[max];
    while (*nd == NULL && max >= 0) {
        max -= 1;
        if (max >= 0)
            *nd = des_noeuds[max];
    }
    return max;
}

SkipList skiplist_insert(SkipList d, int value) {
    // Initialise le noeud courant
    Noeud courant = NULL;
    // ainsi que les tableaux de noeuds suivants et précédents
    Noeud suivants[d->hauteur];
    Noeud precedents[d->hauteur];
    for (unsigned int i = 0; i < d->hauteur; i++) {
        suivants[i] = d->premiers[i];
        precedents[i] = NULL;
    }
    // Récupère le premier noeud non null et la hauteur a laquelle il a été trouvée
    int max = noeud_haut(suivants, &courant, d->hauteur-1);
    while (max >= 0 && courant != NULL) {
        if (courant->valeur > value) {
            // Le noeud courant est trop grand
            max--;
            // Le prochain noeud est celui en dessous
            if (max >= 0)
                courant = suivants[max];
        } else if (courant->valeur < value) {
            // Le noeud courant est plus petit
            max = courant->hauteur-1;
            // Mise à jour des noeuds précédents et suivants
            for (int i = 0; i < value && i < (int)courant->hauteur; i++)
                precedents[i] = courant;
            for (unsigned int j = 0; j < courant->hauteur; j++)
                suivants[j] = courant->suivants[j];
            // Récupère le premier noeud non null et la hauteur a laquelle il a été trouvée
            max = noeud_haut(suivants, &courant, max);
        } else
            // Un noeud de cette valeur existe déjà
            courant = NULL;
    }
    if (max < 0) {
        // Crée le noeud à insérer
        Noeud nouveau = creer_noeud(d, value);
        // Initialise ses noeuds suivants et précédents
        for (unsigned int i = 0; i < nouveau->hauteur; i++) {
            nouveau->suivants[i] = suivants[i];
            nouveau->precedents[i] = precedents[i];
        }
        // Met à jour les noeuds suivants et précédents
        for (unsigned int i = 0; i < nouveau->hauteur; i++) {
            if (nouveau->precedents[i] == NULL)
                d->premiers[i] = nouveau;
            else
                nouveau->precedents[i]->suivants[i] = nouveau;
            if (nouveau->suivants[i] == NULL)
                d->derniers[i] = nouveau;
            else
                nouveau->suivants[i]->precedents[i] = nouveau;
        }
        d->nb_elements += 1;
    }
    return d;
}

bool skiplist_search(SkipList d, int value, unsigned int *nb_operations) {
    Noeud courant;
    Noeud* suivants = d->premiers;
    int max = noeud_haut(suivants, &courant, d->hauteur-1);
    *nb_operations = 1;
    while (max >= 0) {
        if (suivants[max] != NULL) {
            if (suivants[max]->valeur == value) {
                return true;
            }
            else if (suivants[max]->valeur > value)
                max--;
            else {
                max = noeud_haut(suivants, &courant, suivants[max]->hauteur-1);
                suivants = suivants[max]->suivants;
                if (courant != NULL)
                    ++*nb_operations;
            }
        } else
            max--;
    }
    return false;
}

void skiplist_afficher(SkipList sk) {
    Noeud courant = sk->premiers[0];
    printf("Premiers\n");
    for (unsigned int i = 0; i < sk->hauteur; i++)
        printf("%d \n", sk->premiers[i]->valeur);
    while (courant != NULL) {
        printf("\nNoeud %d\n", courant->valeur);
        for (unsigned int i = 0; i < courant->hauteur; i++) {
            if (courant->precedents[i] == NULL)
                printf(" /");
            else
                printf("%2d", courant->precedents[i]->valeur);
            printf(" - ");
            if (courant->suivants[i] == NULL)
                printf(" /\n");
            else
                printf("%2d\n", courant->suivants[i]->valeur);
        }
        courant = courant->suivants[0];
    }
    printf("Derniers\n");
    for (unsigned int i = 0; i < sk->hauteur; i++)
        printf("%d \n", sk->derniers[i]->valeur);
}

SkipListIterator skiplist_iterator_create(SkipList d, unsigned char w) {
    SkipListIterator it = (SkipListIterator)malloc(sizeof(struct s_SkipListIterator));
    it->skiplist = d;
    it->sens = w;
    if (w)
        it->noeud = d->premiers[0];
    else
        it->noeud = d->derniers[0];
    return it;
}

void skiplist_iterator_delete(SkipListIterator it) {
    free(it);
}

SkipListIterator skiplist_iterator_begin(SkipListIterator it) {
    if (it->sens)
        it->noeud = it->skiplist->premiers[0];
    else
        it->noeud = it->skiplist->derniers[0];
    return it;
}

bool skiplist_iterator_end(SkipListIterator it) {
    return it->noeud == NULL;
}

SkipListIterator skiplist_iterator_next(SkipListIterator it) {
    if (!skiplist_iterator_end(it)) {
        if (it->sens)
            it->noeud = it->noeud->suivants[0];
        else
            it->noeud = it->noeud->precedents[0];
    }
    return it;
}

int skiplist_iterator_value(SkipListIterator it) {
    return it->noeud->valeur;
}

SkipList skiplist_remove(SkipList d, int value) {
    Noeud courant;
    Noeud* suivants = d->premiers;
    int max = noeud_haut(suivants, &courant, d->hauteur-1);
    while (max >= 0) {
        if (suivants[max] != NULL) {
            if (suivants[max]->valeur == value) {
                courant = suivants[max];
                for (unsigned int i = 0; i < courant->hauteur; i++) {
                    if (courant->suivants[i] != NULL)
                        courant->suivants[i]->precedents[i] = courant->precedents[i];
                    else
                        d->derniers[i] = courant->precedents[i];
                    if (courant->precedents[i] != NULL)
                        courant->precedents[i]->suivants[i] = courant->suivants[i];
                    else
                        d->premiers[i] = courant->suivants[i];
                }
                detruire_noeud(courant);
                d->nb_elements--;
                return d;
            }
            else if (suivants[max]->valeur > value)
                max--;
            else {
                max = noeud_haut(suivants, &courant, suivants[max]->hauteur-1);
                suivants = suivants[max]->suivants;
            }
        } else
            max--;
    }
    return d;
}
