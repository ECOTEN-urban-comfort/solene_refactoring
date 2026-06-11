/*
 * Copyright 2006, CERMA, Nantes, France.
 */

#ifndef _TC_SOLENE_STRUCTURES_H_
#define _TC_SOLENE_STRUCTURES_H_

struct circuit{
  int nbp;
  int statut;  /* 0:face plane 1:face non plane 2:ligne 2D  3:ligne 3D */
  float transp;
  float visible;
  double fen[6];
  double vnorm[4]; /* A VOIR pourrait etre supprime si on avait le no de la face */
  double *x;                
  double *y;
  double *z;
  struct circuit *suc;
} cir0,cir1;
/* cir0:circuit situe derriere */
/* cir1:circuit situe devant   */

struct contour{
  int *etat;
  struct circuit *debut_support;
  struct circuit *debut_interieur;
  struct contour *suc;
};

struct modelisation_face {
  int nofac_fichier;
  double vnorm[4];
  double fen[6];
  int inverse;
  struct contour *debut_projete  ;
  struct contour *debut_dessin;
}*face;

#endif
