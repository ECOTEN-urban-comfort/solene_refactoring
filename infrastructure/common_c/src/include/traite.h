/*
 * Copyright 2006, CERMA, Nantes, France.
 */

#ifndef _TRAITE_H_
#define _TRAITE_H_

#include "tc_solene_structures.h"

int traite_moins(int,struct modelisation_face*);
int traite_moins_hel(struct modelisation_face*, int, struct modelisation_face*, int);
int devder(int , int , struct modelisation_face *);
int traite_union_nouveau(struct modelisation_face *, int , struct modelisation_face *);
int traite_union(int, struct modelisation_face *);
int traite_inter(int, int);

#endif
