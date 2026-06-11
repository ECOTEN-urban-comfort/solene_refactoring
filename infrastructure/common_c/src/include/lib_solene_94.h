/*
 * Copyright 2006, CERMA, Nantes, France.
 */

#ifndef _LIB_SOLENE_94_H_
#define _LIB_SOLENE_94_H_

#include <tc_solene_structures.h>

#define LONGUEUR_MAXIMALE_CHEMIN 512

int creer_OK_Solene();
int compose_nom_complet(char*, char*, char*, char*);
int compose_nom_complet_sans_ext(char*, char*, char*);
int ajuste_face(int, struct modelisation_face*);
int test_si_nom_existe(char*);
void met_extension_heure(int, char*);

#endif
