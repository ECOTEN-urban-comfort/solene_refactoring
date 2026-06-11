
/* 

cc  -c  solutile.c -lm 
*/
// D. GROLEAU mars 2004 : modification de coplanaire
// D. GROLEAU MODIF dec 2005 ; modifie de coplanaire (sauvegarde le d du plan avant calcul et restore après)

/*  les commentaires avec $$ et && sont a modifier si on veut tester les  */
/*  allocations - il faut en plus enlever les commentaires dans solene.n  */
/*  sur les declarations des vecteurs de pointeurs (rechercher$$)         */


  /*____________________________________________________________*/
  /*      ALLOCATION - DESALLOCATION - VERIFICATION		        */
  /*        LECTURE ECRITURE DU FICHIER SUR DISQUE	            */
  /*                    COPIE				                    */
  /*                 INITIALISATION			                    */
  /*                 UTILITAIRES                                */
  /*		  VERIFICATION-LISTE	                            */
  /*       FENETRE ENGLOBANTE ET NORMALISATION	                */
  /*      CALCUL ET OPERATIONS SUR LES FENETRES                 */
  /*         TRANSFORMATIONS GEOMETRIQUES                       */
  /*             SPHERE ET ELLIPSE			                    */
  /*             		                                        */
  /*            TRAITE VU/CACHE pour faces coplanaires			*/
  /*             		                                        */
  /*    tableau de VISIBILITE (observateur, scene triangulee    */
  /*____________________________________________________________*/


#include<solene.h>

// DECLARATION FUNCTIONS

int z_axono_pers=0;   /* option calcul du z : axono par defaut ;=1 pers */
double ang_ellipse();

/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*      ALLOCATION - DESALLOCATION - VERIFICATION		      */
/*____________________________________________________________________*/
/*____________________________________________________________________*/
double *alloue_double(nb,no)
int nb,no;
{
double *pt;
int i;

  	pt=(double *)malloc(nb*sizeof(double));
	if(pt==NULL)
		{printf("\n pb allocation alloue_double %d \n",no);
		 exit(0);
		}
	for(i=0;i<nb;i++) pt[i]=0;
 	return(pt);
}
/*____________________________________________________________________*/
int desalloue_double(pt)
double *pt;
{
	free(pt);
}
/*____________________________________________________________________*/
float *alloue_float(nb,no)
int nb,no;
{
float *pt;
int i;

	pt=(float *)malloc(nb*sizeof(float));
	if(pt==NULL)
		{printf("\n pb allocation alloue_float %d \n",no);
		 exit(0);
		}
	for(i=0;i<nb;i++) pt[i]=0;
 	return(pt);
}
/*____________________________________________________________________*/
int desalloue_float(pt)
float *pt;
{
	free(pt);
}
/*-----------------------------------------------------------------------------*/
int *alloue_int(nb,no)
int nb,no;
{
int *pt,i;

  	pt=(int *)malloc(nb*sizeof(int));
	if(pt==NULL)
		{printf("\n pb allocation alloue_int %d \n",no);
		 exit(0);
		}
	for(i=0;i<nb;i++) pt[i]=0;
	return(pt);

}
/*____________________________________________________________________*/
int desalloue_int(pt)
int *pt;
{
	free(pt);
}
/*-----------------------------------------------------------------------------*/

struct modelisation_face *alloue_face(nbf,err)
int nbf,err;
{    /* alloue et initialise nbf faces */
  struct modelisation_face *face;
  int i;
     face=(struct modelisation_face *)malloc(nbf*sizeof(*face));
     if(face==NULL) pb_allocation(err); 
     for(i=0;i<nbf;i++)
         {face[i].debut_projete=NULL; face[i].debut_dessin=NULL; } 
/* 
     i=0;
     while(pteur_face[i])
         {if(i>=NBFACEMAX){printf("\n nb contour >%d",NBFACEMAX);exit(0);}
          i++;
         }
     pteur_face[i]=face;
  */
     return(face);
}
/*_______________________________________________________________________*/

struct spol *alloue_poly(nbp,no)
int nbp;
int no;
{int i;
  struct spol *poly;

       /* alloue et initialise poly */
  poly=(struct spol *)malloc(sizeof(*poly));
  if(poly!=NULL)
    { poly->nbp=0;
      poly->statut_point=(char *)malloc(nbp*sizeof(char));
      poly->statut_arete=(char *)malloc(nbp*sizeof(char));
      poly->traite_arete=(char *)malloc(nbp*sizeof(char));
      poly->x=(double *)malloc(nbp*sizeof(double));
      poly->y=(double *)malloc(nbp*sizeof(double));
      poly->z=(double *)malloc(nbp*sizeof(double));
/* 
     i=0;
     while(pteur_poly[i])
         {if(i>=NBPOLYMAX){printf("\n nb contour >%d",NBPOLYMAX);exit(0);}
          i++;
         }
     pteur_poly[i]=poly;

  */
     return(poly);
    }
  else { printf(" impossible alloue poly no = %d \n",no); exit(0); }


}
/*_______________________________________________________________________*/

int desalloue_poly(poly)
struct spol *poly;
{int i;

/* 
  i=verif_poly(poly); pteur_poly[i]=0;
  */

  free(poly->statut_point);free(poly->statut_arete);
  free(poly->traite_arete);free(poly->x);free(poly->y);free(poly->z);
  free(poly);
}

/*_______________________________________________________________________*/
int desalloue_face(face)
struct modelisation_face *face;
{int i;


  if(face)
    { desalloue_chaine_contour(face->debut_projete,1032); face->debut_projete=NULL;
      desalloue_chaine_contour(face->debut_dessin,1033); face->debut_dessin=NULL;
/* 
  i=verif_face(face); pteur_face[i]=0;
  */
      free(face);
    }
}
/*_______________________________________________________________________*/
int desalloue_fface(face,nbf)
struct modelisation_face *face;
int nbf;
{int i;


  if(face)
    { for(i=0;i<nbf;i++)
      {desalloue_chaine_contour((face+i)->debut_projete,1034);
       (face+i)->debut_projete=NULL;
       desalloue_chaine_contour((face+i)->debut_dessin,1035);
       (face+i)->debut_dessin=NULL;
      }
/* 
  i=verif_face(face); pteur_face[i]=0;
  */
    free(face);
    }
}
/*_______________________________________________________________________*/
int desalloue_contour_face(face)
struct modelisation_face *face;
{
  if(face)
    { desalloue_chaine_contour(face->debut_projete,1036);
       face->debut_projete=NULL;
      desalloue_chaine_contour(face->debut_dessin,1037);
       face->debut_dessin=NULL;
   }
}

/*_______________________________________________________________________*/

int pb_allocation(no)
int no;
{  printf("\n pb allocation %d - on arrete ",no);   exit(0);
}
/*_______________________________________________________________________*/

int verif_contour(pcont,no)
struct contour *pcont;
int no;
{int i;


 i=0;
/* 
 while(pteur_contour[i]!=pcont)
      {i++;if(i==(NBCONTMAX-1))
            {printf("\n desalloue contour pointeur %ld n'existe pas-appel %d",pcont,no);
             exit(0);
            }
      }
  */
 return(i);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int verif_circuit(pcir,no)
int no;
struct circuit *pcir;
{int i;

 i=0;
/* 
 while(pteur_circuit[i]!=pcir)
  {i++;if(i==(NBCIRMAX-1))
     {printf("\n  circuit pointeur %ld n'existe pas - appel %d",pcir,no);exit(0);}
  }
  */
 return(i);

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int verif_point(pt,no)
struct point *pt;
int no;
{int i;

 i=0;
/* 
while(pteur_point[i]!=pt)
      {i++;if(i==(NBPOINT-1))
            {printf("\n desalloue point pointeur %ld n'existe pas-appel %d ",pt,no);exit(0);}}
  */

 return(i);

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int verif_circuit_point(p,no)
double *p;
int no;
{int i;


 i=0;
/* 
 while(pteur_circuit_point[i]!=p)
    {i++;if(i==(NBPTCIRMAX-1))
          {printf("\n desalloue point circuit pointeur %ld n'existe pas-appel %d",p,no);exit(0);}
    }
  */

return(i);

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int verif_face(p)
struct modelisation_face *p;
{int i;

 i=0;
/* 
 while(pteur_face[i]!=p)
    {i++;if(i==(NBFACEMAX-1))
          {printf("\n desalloue face pointeur %ld n'existe pas-a",p);exit(0);}
    }
  */

return(i);

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int verif_poly(p)
struct spol *p;
{int i;

 i=0;
/* 
 while(pteur_poly[i]!=p)
    {i++;if(i==(NBPOLYMAX-1))
          {printf("\n desalloue poly pointeur %ld n'existe pas",p);exit(0);}
    }
  */
return(i);

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
int verif_liste_point(p,no)
struct liste_point *p;
int no;
{int i;

 i=0;
/* 
 while(pteur_liste_point[i]!=p)
    {i++;if(i==(NBLISTEPOINT-1))
          {printf("\n desalloue liste_point pointeur %ld n'existe pas no = %d ",p,no);exit(0);}
    }
  */

return(i);

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int verif_inters(p)
struct inters *p;
{int i;

 i=0;
/* 
 while(pteur_inters[i]!=p)
    {i++;if(i==(NBINTERS-1))
          {printf("\n desalloue inters pointeur %ld n'existe pas",p);exit(0);}
    }
  */
return(i);

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/


int alloue_point_circuit(pcir,no)
struct circuit *pcir;
int no;
{int i,j;
   pcir->x=(double *)malloc((pcir->nbp)*(sizeof(double)));
   pcir->y=(double *)malloc((pcir->nbp)*(sizeof(double)));
   pcir->z=(double *)malloc((pcir->nbp)*(sizeof(double)));
   if(pcir->x==NULL || pcir->y==NULL || pcir->z==NULL)
      {printf("\n alloue_point_circuit pb allocation nbp = %d erreur no %d\n ",pcir->nbp,no);exit(0);}

/* 
   for(i=0;i<NBPTCIRMAX;i++)
     {if(pteur_circuit_point[i]==0){pteur_circuit_point[i]=pcir->x; j=i; break;}}
   for(i=j;i<NBPTCIRMAX;i++)
      {if(pteur_circuit_point[i]==0){pteur_circuit_point[i]=pcir->y; j=i; break;}}
   for(i=j;i<NBPTCIRMAX;i++)
      {if(pteur_circuit_point[i]==0){pteur_circuit_point[i]=pcir->z; j=i; break;}}
  if(i==NBPTCIRMAX){printf("\n plus de place dans pteur_circuit_point "); exit(0);}
  */

for(i=0;i<pcir->nbp;i++)
	{pcir->x[i]=0; pcir->y[i]=0; pcir->z[i]=0;}

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct circuit *alloue_circuit(no)
int no;
{struct circuit *pcir;
 int i;

    pcir=(struct circuit *)malloc(sizeof(*pcir));
    if(pcir!=NULL)
        {init_cir(pcir);}
    else {printf("\n pb allocation alloue_circuit no %d\n",no);
          exit(0);
         }
/* 
        i=0;
        while(pteur_circuit[i])
            {if(i>=NBCIRMAX){printf("\n nb circuit >NBCIRMAX");exit(0);}
             i++;
            }
          pteur_circuit[i]=pcir;
  */

   /*    printf(" alloue circuit no %d pt = %ld \n",no,pcir); */

    return(pcir);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct point *alloue_point()
{struct point *pt;
 int i;

  pt=(struct point *)malloc(sizeof(*pt));
  if(pt!=NULL)
     {pt->x=0; pt->y=0; pt->z=0; pt->statut=0;
     }
  else {printf("\n pb allocation alloue_point \n");
       exit(0);
      }
/* 
      i=0;
      while(pteur_point[i])
          {if(i>=NBPOINT){printf("\n nb point >NBPOINT");exit(0);}
           i++;
          }
      pteur_point[i]=pt; 
  */

  return(pt);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct contour *alloue_contour(no)
int no;
{struct contour *pc;
 int i;
  pc=(struct contour *)malloc(sizeof(*pc));
  if(pc!=NULL)
    {init_res(pc);}
  else {printf("\n pb allocation alloue_contour no %d \n",no);
       exit(0);
      }
/* 
     i=0;
     while(pteur_contour[i])
         {if(i>=NBCONTMAX){printf("\n nb contour >%d",NBCONTMAX);exit(0);}
          i++;
         }
     pteur_contour[i]=pc;
  */

/*  printf(" alloue contour no %d pt = %ld \n",no,pc); */

  return(pc);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/


int desalloue_point(pt,no)
struct point *pt;
int no;
{ int i;

/* 
  i=0;
  i=verif_point(pt,no); 
  pteur_point[i]=0;
  */

  free(pt);  
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_contour(pcont,no)
struct contour *pcont;
int no;
{int i;

/* 
 i=0;
 i=verif_contour(pcont,no);
 pteur_contour[i]=0;
  */

 if(nb_etat)free(pcont->etat);

 desalloue_chaine_circuit(pcont->debut_support,no);
 desalloue_chaine_circuit(pcont->debut_interieur,no);

  free(pcont);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_chaine_contour(pcont,no)
struct contour *pcont;
int no;
{struct contour *pcontsuc;
 if(pcont==NULL)return(0);
 while(pcont)
 {pcontsuc=pcont->suc;
  desalloue_contour(pcont,no);
  pcont=pcontsuc; 
 }
}
/*____________________________________________________________________*/
int libere_contour(pcont,no)
struct contour *pcont;
int no;
{int i;

/* 
i=0;
 i=verif_contour(pcont,no);
 pteur_contour[i]=0;
  */

 if(nb_etat)free(pcont->etat);

  free(pcont);

  
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_circuit(pcir,no)
struct circuit *pcir;
int no;
{int i;

/* 
  i=0;
  i=verif_circuit(pcir,no);
  pteur_circuit[i]=0;
  */
	 desalloue_point_circuit(pcir,2);
	 free(pcir); pcir=NULL;

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_chaine_circuit(pcir,no)
struct circuit *pcir;
int no;
{struct circuit *psuc;


 while(pcir)
	{psuc=pcir->suc;
	 desalloue_circuit(pcir,no);
	 pcir=psuc;
	}
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_point_circuit(pcir,no)
struct circuit *pcir;
int no;
{ int i;

if(pcir->nbp)
      {
/* 
       i=verif_circuit_point(pcir->x,no); pteur_circuit_point[i]=0;
       i=verif_circuit_point(pcir->y,no); pteur_circuit_point[i]=0;
       i=verif_circuit_point(pcir->z,no); pteur_circuit_point[i]=0;
  */
	free(pcir->x);free(pcir->y);free(pcir->z);
     }
pcir->nbp=0;

  pcir->x=NULL;  pcir->y=NULL;  pcir->z=NULL;
}
/*____________________________________________________________________*/

int desalloue_point_circuit_cir(pcir,no)
struct circuit *pcir;
int no;
{ int i;

if(pcir->nbp)
      {
/* 
       i=verif_circuit_point(pcir->x,no); pteur_circuit_point[i]=0;
       i=verif_circuit_point(pcir->y,no); pteur_circuit_point[i]=0;
       i=verif_circuit_point(pcir->z,no); pteur_circuit_point[i]=0;
  */
	free(pcir->x);free(pcir->y);free(pcir->z);
     }
pcir->nbp=0;

  pcir->x=NULL;  pcir->y=NULL;  pcir->z=NULL;
}

/*____________________________________________________________________*/

struct valeur_face *alloue_valeur(nbf,err)
int nbf,err;
{    /* alloue et initialise nbf faces */
  struct valeur_face *valeur;
  int i;
     valeur=(struct valeur_face *)malloc(nbf*sizeof(*valeur));
     if(valeur==NULL) pb_allocation(err); 
     for(i=0;i<nbf;i++)valeur[i].val=0;  
     return(valeur);
}
/*_______________________________________________________________________*/
int desalloue_valeur(valeur)
struct valeur_face *valeur;
{
   if(valeur) free(valeur);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct liste_point *alloue_liste_point(no)
int no;
{struct liste_point *lcir;
 int i;

  lcir=(struct liste_point *)malloc(sizeof(*lcir)); 
  if(lcir==NULL)
      {printf("\n pb allocation alloue_liste_point no %d \n",no);
       exit(0);
      }
/* 
     i=0;
     while(pteur_liste_point[i])
         {if(i>=NBLISTEPOINT){printf("\n nb liste_point >%d",NBLISTEPOINT);exit(0);}
          i++;
         }
     pteur_liste_point[i]=lcir;
  */

     lcir->suc=NULL;

  return(lcir);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_liste_point(lc,no)
struct liste_point *lc;
int no;
{int i;
struct liste_point *lcir,*lcirsuc;


 lcir=lc;
 while(lcir)
   {lcirsuc=lcir->suc; 
/* 
    i=0;
    i=verif_liste_point(lcir,no);
    pteur_liste_point[i]=0;
  */

    free(lcir); lcir=lcirsuc;
   }
 

}
/*____________________________________________________________________*/
int libere_liste_point(lcir,no)
struct liste_point *lcir;
int no;
{int i;
i=0;

/* 
 i=verif_liste_point(lcir,no);
 pteur_liste_point[i]=0;
  */

  free(lcir);
  
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct inters *alloue_inters(no)
int no;
{struct inters *lci;
 int i;

  lci=(struct inters *)malloc(sizeof(*lci)); 
  if(lci==NULL)
      {printf("\n pb allocation alloue_inters no %d \n",no);
       exit(0);
      }
/* 
     i=0;
     while(pteur_inters[i])
         {if(i>=NBINTERS){printf("\n nb inters >%d",NBINTERS);exit(0);}
          i++;
         }
     pteur_inters[i]=lci;
  */

     lci->suc=NULL;
  return(lci);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int desalloue_inters(no)
int no;
{int i;
 struct inters *li,*lisuc;

 if(debut_inter==NULL)return;
  li=debut_inter;
   while(li)
     {lisuc=li->suc; 
/* 
      i=0;
      i=verif_inters(li,no);
      pteur_inters[i]=0;
  */
      free(li); li=lisuc;
     }
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*        LECTURE ECRITURE DU FICHIER SUR DISQUE	             */
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int lit_en_tete(fp,nbfac,numax,englob)
FILE *fp;
int *nbfac,*numax;
double *englob;
{int i;
  fscanf(fp,"%d %d",nbfac,numax);
  for(i=0;i<10;i+=2)fscanf(fp,"%lf %lf",englob+i,englob+i+1);

}
/*____________________________________________________________________*/
int ecrit_en_tete(fp,nbfac,numax,englob)
FILE *fp;
int nbfac,numax;
double *englob;
{int i;
  fprintf(fp,"%7d %7d\n",nbfac,numax);
  for(i=0;i<10;i+=2)fprintf(fp,"%15.3lf %15.3lf\n",englob[i],englob[i+1]);

}

/*_________________________________________________________________*/
int lit_fic_etat(pfic,f1,nbf1,no_etat,proj)
FILE *pfic;
struct modelisation_face *f1;
int nbf1,no_etat,proj;
{ char c;
 int k,kk,nbcont,i,j;
 struct contour *pc;

 for(k=0;k<nbf1;k++)
	{/*printf("\n face no %d  nofac %d nbcont  =  \n",k,(f1+k)->nofac_fichier);*/
	 fscanf(pfic,"\n%c%d %d",&c,&kk,&nbcont);
 	 if(proj)pc=(f1+k)->debut_projete;
	 else pc=(f1+k)->debut_dessin;
         for(i=0;i<nbcont;i++)
            {fscanf(pfic,"\n%c",&c);
	     for(j=0;j<no_etat;j++)
	         {fscanf(pfic,"%d",&kk);
		  pc->etat[j]=kk; ;
                 }
  	     pc=pc->suc;
            }
        }
/*
for(i=0;i<nbfmax;i++)
	{printf("\n face no %d nofac = %d ",i,(f1+i)->nofac_fichier);
         liste_face(f1+i,1); 
         liste_etat(f1+i,1,no_etat);
        }
*/
}

/*_________________________________________________________________*/
/* Lecture d'un fichier  (.val) et remplissage tableau 'valeur' */
void lect_fic_val(pfic, valeur)
FILE *pfic;
double *valeur;
{ double val_min;
  double val_max;
  int num_cont, num_face, num_cont_liste;
  int nbfac,nomax;
  int nofac, nbcont_face;
  char c;

  fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);
  num_cont_liste = 0;
  for(num_face=0;num_face<nbfac;num_face++)
	{
	fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont_face);
	for(num_cont=0;num_cont<nbcont_face;num_cont++)	
		{	
		fscanf(pfic,"%lf\n",valeur+num_cont_liste);
		num_cont_liste++;
		}
	}
}

/*_______________________________________________________________________*/
int lit_fic_valeur(fp,nbf,valeur)
FILE *fp;
int nbf;
struct valeur_face *valeur;
{char c;
 int k,nofac;
 double val;
 for(k=0;k<nbf;k++)
    { fscanf(fp,"\n%c%d %lf",&c,&nofac,&val);
      (valeur+k)->nofac=nofac;  (valeur+k)->val=val;
    }
}
/*_______________________________________________________________________*/
int affecte_fic_valeur(nbf,valeur,valno,coef)
int nbf;
struct valeur_face *valeur,*valno;
double coef;
{char c;
 int k,nofac;
 double val;
 for(k=0;k<nbf;k++)
    {(valeur+k)->nofac=(valno+k)->nofac;  (valeur+k)->val=coef;
    }
}

/*-----------------------------------------------------------*/
int output_valeur_sur_fichier(valeur,nb,pf,nbf,nomax)
struct valeur_face *valeur;
int nb,*nomax,*nbf;
FILE *pf;
{int i;
 *nomax=0;   *nbf=nb;
 for(i=0;i<nb;i++) 
    { if(valeur[i].nofac>*nomax)*nomax=valeur[i].nofac;
      if(valeur[i].nofac!=-999.999)
             fprintf(pf,"f%d %lf\n",valeur[i].nofac,valeur[i].val);
      else *nbf=*nbf-1;
    }
}
/*-----------------------------------------------------------*/
int lit_fic_cir3d(fp,nbf,face)
FILE *fp;
int nbf;
struct modelisation_face *face;
{     
 int i,j,k,kk,ic,nofac,nbcir,nbtrou,nbface;
 struct contour *pcont;        
 struct circuit *pcir; 
 double xn,yn,zn,nor[3];
 char c;

   /* lit ,sur fichier fp, nbf faces et met ds face alloue en bloc */
                          
 for(kk=0;kk<nbf;kk++)
    { fscanf(fp,"\n%c%d %d",&c,&nofac,&nbcir);     
      /* printf(" lit face no %d nb contour = %d\n",nofac,nbcir); */
      fscanf(fp,"%lf %lf %lf",&xn,&yn,&zn);
      face[kk].nofac_fichier=nofac;
      face[kk].vnorm[0]=xn;face[kk].vnorm[1]=yn;face[kk].vnorm[2]=zn;
      
         /* lit les contours */
      for(i=0;i<nbcir;i++)
       {  
            fscanf(fp,"\n%c%d",&c,&nbtrou);
            /*    printf(" lit contour  nbtrou = %d\n",nbtrou); */
	    /* lit le contour support */
           
            if(i==0)
               { face[kk].debut_projete=alloue_contour(1002);
                 pcont=face[kk].debut_projete;
               }
            else
               { pcont->suc=alloue_contour(1008);
                 pcont=pcont->suc;
               }
            pcont->suc=NULL;  
            pcir=alloue_circuit(1003);
            pcont->debut_support=pcir;
            pcir->suc=NULL;
            for(k=0;k<3;k++)pcir->vnorm[k]=face[kk].vnorm[k];
            fscanf(fp,"%d",&(pcir->nbp));
            /*printf("  nb de pts %d\n",pcir->nbp);*/
            alloue_point_circuit(pcir,1004);
            for(k=0;k<pcir->nbp;k++)
                 {fscanf(fp,"%lf%lf%lf",&(pcir->x[k]),&(pcir->y[k]),&(pcir->z[k]));}
            fenetre_circuit(pcir);  

             /* lit les trous du contour support */

	    for(j=0;j<nbtrou;j++)
              { if(j==0)
                     { pcir=alloue_circuit(1005);
                       pcont->debut_interieur=pcir;
                     } 
                else { pcir->suc=alloue_circuit(1006); 
                       pcir=pcir->suc; 
                     }
                pcir->suc=NULL;
                fscanf(fp,"\n%c",&c);
                for(k=0;k<3;k++)pcir->vnorm[k]=face[kk].vnorm[k];
                fscanf(fp,"%d",&(pcir->nbp));
                alloue_point_circuit(pcir,1007);
                for(k=0;k<pcir->nbp;k++)
                  {fscanf(fp,"%lf%lf%lf",&(pcir->x[k]),&(pcir->y[k]),&(pcir->z[k]));}
                fenetre_circuit(pcir);
	      }   
      }
     fen_face(face+kk,1);
   }
}

/*-----------------------------------------------------------*/
int output_face_sur_fichier(face,nb,projete,itranf,pf,nbf,nomax)
struct modelisation_face *face;
int nb,projete,itranf,*nbf,*nomax;
FILE *pf;
{
              /* ecrit sur fichier pf,les nb faces; contour projete si projete=1; */
              /*                                    contour dessin  si projete=1; */
              /* en appliquant Tranfo inverse_Normalise  si itranf */
              /* et en mettant a jour nb de faces du fichier, nb contour, nomax */
  struct contour *pcont;
  struct circuit *pcir;
  int i,nbcont,nbtrou;



  for(i=0;i<nb;i++) 
    {/* met sens circuits en accord avec sens de la normale */

/* APPEL COURCIRCUITE D. GRoleau 23 juillet 1996 */
/* car procedure incorrecte ? */

/*      test_si_inv_sens(face+i,projete); */

/* FIN COURCIRCUIT */

     if(projete) pcont=face[i].debut_projete; else pcont=face[i].debut_dessin;
     if(pcont)
       {*nbf=*nbf+1; if(face[i].nofac_fichier>*nomax) *nomax=face[i].nofac_fichier;
        nbcont=nb_contour_face(face+i,projete);
        fprintf(pf,"f%d %d\n",face[i].nofac_fichier,nbcont);
        fprintf(pf," %lf  %lf  %lf\n",face[i].vnorm[0],face[i].vnorm[1],face[i].vnorm[2]);
 
        while(pcont)
         { nbtrou=nb_trou_contour(pcont);
           pcir=pcont->debut_support;
                       /* support ; en principe 1 seul */
           fprintf(pf,"c%d\n",nbtrou);
           output_circuit_sur_fichier(pcir,pf,itranf);
           pcir=pcont->debut_interieur;
                      /* les trous */
           while(pcir)
              { fprintf(pf,"t%\n");
                output_circuit_sur_fichier(pcir,pf,itranf);
                pcir=pcir->suc;
              }
           pcont=pcont->suc;
        }
     }
   }
}

/*-----------------------------------------------------------*/
int output_circuit_sur_fichier(pcir,pf,itranf,inv)
struct circuit *pcir;
int itranf;
FILE *pf;
{
 int i;
 double xx,yy,zz;
              /* ecrit sur fichier pf le circuit  */
              /* en appliquant Tranfo inverse Normalise  si itranf */
   fprintf(pf,"    %d\n",pcir->nbp);
   for(i=0;i<pcir->nbp;i++)
     { if(itranf)
         { denormalise_point(pcir->x[i],pcir->y[i],pcir->z[i],&xx,&yy,&zz);
           fprintf(pf," %lf  %lf  %lf\n",xx,yy,zz);
         }
       else fprintf(pf," %lf  %lf  %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
     }
}



/*_________________________________________________________________*/
/* en COMMENTAIRE */
/*
ecrit_circuit(p,fp)
struct circuit *p;
FILE *fp;
{int i;
  fprintf(fp," %d\n",p->nbp);
  for(i=0;i<p->nbp;i++)
     fprintf(fp,"     %15.7lf %15.7lf %15.7lf\n",p->x[i],p->y[i],p->z[i]);
}
*/

/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*                    COPIE					      */
/*____________________________________________________________________*/
/*____________________________________________________________________*/

struct contour *copie_contour(pcont,pc)
struct contour *pcont,*pc;
{struct circuit *pcir,*pci,*pcour;
 int i;

   if(pc!=NULL)desalloue_contour(pc,213);
   pc=alloue_contour(213);
   for(i=0;i<nb_etat;i++)pc->etat[i]=pcont->etat[i];
   pcir=pcont->debut_support;
   while(pcir)
     {pci=alloue_circuit(214);
      copie_cir(pcir,pci);
      if(pc->debut_support==NULL)pc->debut_support=pci;
      else pcour->suc=pci;
      pcour=pci;
      pcir=pcir->suc;
     }
   pcir=pcont->debut_interieur;
   while(pcir)
     {pci=alloue_circuit(214);
      copie_cir(pcir,pci);
      if(pc->debut_interieur==NULL)pc->debut_interieur=pci;
      else pcour->suc=pci;  
      pcour=pci;
      pcir=pcir->suc;
     }

 return(pc);
}

/*____________________________________________________________________*/

int copie_cir(pocir,pcir)
struct circuit *pocir,*pcir;
{int i;
 if(pcir->x!=NULL)desalloue_point_circuit(pcir,5);
  *pcir=*pocir; pcir->suc=NULL;
   alloue_point_circuit(pcir,9);
/* ajouter le 2/04/93 */
   pcir->vnorm[0]=pocir->vnorm[0];  pcir->vnorm[1]=pocir->vnorm[1];
   pcir->vnorm[2]=pocir->vnorm[2];  pcir->vnorm[3]=pocir->vnorm[3];

  for(i=0;i<pcir->nbp;i++)
      {pcir->x[i]=pocir->x[i]; pcir->y[i]=pocir->y[i]; pcir->z[i]=pocir->z[i];}
 }
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int cirsol(pciro,pciri)
struct circuit *pciro,*pciri;
{int i;
  for(i=0;i<pciri->nbp;i++)
     {pciri->x[i]=pciro->x[i]; pciri->y[i]=pciro->y[i];
      pciri->z[i]=pciro->z[i];
     }
}

/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/

int copie_projete_dessin(nodeb,nofin)
int nodeb,nofin;
{int i;

 for(i=nodeb;i<nofin;i++)copie_face_projete(&face[i],1,&face[i],0);
}
/*_______________________________________________________________________*/
int copie_face(face1,face2)
struct modelisation_face *face1,*face2;
{ 
 struct contour *pc1,*pc2;
   /* copie face1 dans face2 */
   desalloue_chaine_contour(face2->debut_projete,1038);
   desalloue_chaine_contour(face2->debut_dessin,1039);
   *face2=*face1; 
   pc1=face1->debut_projete;
   pc2=NULL;
   while(pc1)
     { if(pc2==NULL) { pc2=copie_contour(pc1,pc2);
                       face2->debut_projete=pc2;
                     }
       else
         { pc2->suc=copie_contour(pc1,pc2->suc);pc2=pc2->suc; }
       pc1=pc1->suc; 
     }
  
   pc1=face1->debut_dessin;
   pc2=NULL;
   while(pc1)
     { if(pc2==NULL)
         { pc2=copie_contour(pc1,pc2); face2->debut_dessin=pc2; }
       else
         { pc2->suc=copie_contour(pc1,pc2->suc);pc2=pc2->suc; }
       pc1=pc1->suc; 
     }
   
}
/*_______________________________________________________________________*/
int copie_face_projete(face1,proj1,face2,proj2)
struct modelisation_face *face1,*face2;
int proj1,proj2;
{ int i;
 struct contour *pc1,*pc2;
   if(face1==face2 && proj1==proj2)return(0);
   if(proj2){desalloue_chaine_contour(face2->debut_projete,1040); face2->debut_projete=NULL;}
   else {desalloue_chaine_contour(face2->debut_dessin,1041); face2->debut_dessin=NULL;}

  face2->nofac_fichier=face1->nofac_fichier;
  for(i=0;i<4;i++)face2->vnorm[i]=face1->vnorm[i];
  for(i=0;i<6;i++)face2->fen[i]=face1->fen[i];
  
  face2->inverse=face1->inverse;

   if(proj1)pc1=face1->debut_projete;
   else pc1=face1->debut_dessin;

   pc2=NULL;
   while(pc1)
     { if(pc2==NULL) { pc2=copie_contour(pc1,pc2); 
                       if(proj2)face2->debut_projete=pc2;
		       else face2->debut_dessin=pc2;
                     }
       else
         { pc2->suc=copie_contour(pc1,pc2->suc);pc2=pc2->suc;}
       pc1=pc1->suc; 
     }
  
}
/*_______________________________________________________________________*/
int egal_face(face1,face2)
struct modelisation_face *face1,*face2;
{  
  /* fait face1=face2 */
  /* on desalloue les contours de face1 auparavant */
  desalloue_chaine_contour(face1->debut_projete,1042);
  desalloue_chaine_contour(face1->debut_dessin,1043);
  /* fait face1=face2 */
  *face1=*face2;
  /* on vide face2 de ses contours */
  face2->debut_projete=NULL; face2->debut_dessin=NULL;
}

/*_______________________________________________________________________*/
int concatene_fface(face1,proj1,face2,proj2)
struct modelisation_face *face1,*face2;
int proj1,proj2;
{        /* ajoute contours de face 1 a face 2 */
 struct contour *pcontdeb,*pcont,*pcontprec;
  if(proj1)
    {pcont=face1->debut_projete;
     face1->debut_projete=NULL;
    }
  else
    {pcont=face1->debut_dessin;
     face1->debut_dessin=NULL;
    }
 if(pcont==NULL)return;

 if(proj2)
    { pcontdeb=face2->debut_projete;
      face2->debut_projete=pcont; 
    }
  else
    { pcontdeb=face2->debut_dessin; 
      face2->debut_dessin=pcont;
    }

  while(pcont) { pcontprec=pcont; pcont=pcont->suc; }
  pcontprec->suc= pcontdeb;
  fen_face(face2,proj2); 
}


/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*                 INITIALISATION				      */
/*____________________________________________________________________*/
/*____________________________________________________________________*/


int init_cir(pcir)
struct circuit *pcir;
{int i;
 pcir->nbp=0;  pcir->transp=0;  pcir->visible=0;
 for(i=0;i<5;i++)pcir->fen[i]=0;  for(i=0;i<3;i++)pcir->vnorm[i]=0;
 pcir->x=NULL; pcir->y=NULL; pcir->z=NULL;  pcir->suc=NULL;
}
/*____________________________________________________________________*/

int init_fenetre_affichage()
{int i;
 for(i=0;i<3;i++)fen_aff[i]=100000000000.;   for(i=3;i<6;i++)fen_aff[i]=-1000000000000.;
}
/*____________________________________________________________________*/

init_verif_pteur_solene()
{int i;

/* 
  for(i=0;i<NBCIRMAX;i++)pteur_circuit[i]=NULL;
  for(i=0;i<NBCONTMAX;i++)pteur_contour[i]=NULL;
  for(i=0;i<NBPTCIRMAX;i++)pteur_circuit_point[i]=NULL;
  for(i=0;i<NBPOINT;i++)pteur_point[i]=NULL;
  for(i=0;i<NBPOLYMAX;i++)pteur_poly[i]=NULL;
  for(i=0;i<NBFACEMAX;i++)pteur_face[i]=NULL;
  for(i=0;i<NBINTERS;i++)pteur_inters[i]=NULL;
  for(i=0;i<NBLISTEPOINT;i++)pteur_liste_point[i]=NULL;
  */

}

/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
int init_res(res)
struct contour *res;
{ int i;
  if(nb_etat)
    {res->etat=(int *)malloc(nb_etat*sizeof(int));
     if(res->etat==NULL)
          {printf("\n pb allocation % etats d'un contour \n");exit(0);}
     for(i=0;i<nb_etat;i++)res->etat[i]=0;
    }
   else res->etat=NULL;
   res->debut_support=NULL; res->debut_interieur=NULL; res->suc=NULL;
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*                   UTILITAIRES                                      */
/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*_________________________________________________________________*/
int lit_intervalle(chaine,cara,x,y)
char *chaine,cara;
double *x,*y;
{ char s1[10],s2[10],*s,*ch;

        *s1=0; *s2=0;
        s=s1;ch=chaine;
        while(*ch)
          {if(*ch==cara){*s=0; s=s2;}
           else {*s=*ch;s++;}
            ch++;
          }
        *s=0;
        sscanf(s1,"%lf",x);
        if(*s2){sscanf(s2,"%lf",y);return(2);}
        else return(1);
}
/*_________________________________________________________________*/
int test_egalite_de_2_chaines(ch1,ch2)
char *ch1,*ch2;
{ 

        while(*ch1)
          {if(*ch1!=*ch2)return(0);
            ch1++;ch2++;
          }
       if(*ch2==0)return(1);
       else return(0);
}

/*-----------------------------------------------------------*/
int sauve_par_aff()
{
 int i;
 save_obs_x=obs.x; save_obs_y=obs.y; save_obs_z=obs.z;
 save_obs_xo=obs.xo; save_obs_yo=obs.yo; save_obs_zo=obs.zo;
 for(i=0;i<8;i++)save_fen_aff[i]=fen_aff[i];
 for(i=0;i<9;i++)save_obs_tran[i]=obs.tran[i];
}

/*-----------------------------------------------------------*/
int restore_par_aff()
{
 int i;
 obs.x=save_obs_x; obs.y=save_obs_y; obs.z=save_obs_z;
 obs.xo=save_obs_xo; obs.yo=save_obs_yo; obs.zo=save_obs_zo;
 for(i=0;i<8;i++)fen_aff[i]=save_fen_aff[i];
 for(i=0;i<9;i++)obs.tran[i]=save_obs_tran[i];
}

/*-----------------------------------------------------------*/
int z_min_max_face(face,zmin,zmax)
struct modelisation_face *face;
double *zmin,*zmax;
{ 
  struct contour *pcont;
  struct circuit *pcir;
  int i;
  double zmi,zma;
  zmi=100000000000.; zma=-100000000000000.;
          /* cherche mi et max dez de face  sur projete */
   pcont=face->debut_projete;
   while(pcont)
    { pcir=pcont->debut_support;     
      for(i=0;i<pcir->nbp-1;i++)
          { zmi=dmin(zmi,pcir->z[i]);
            zma=dmax(zma,pcir->z[i]);
          }
      pcir=pcont->debut_interieur;
      while(pcir)
        { for(i=0;i<pcir->nbp-1;i++)
            { zmi=dmin(zmi,pcir->z[i]);
              zma=dmax(zma,pcir->z[i]);
            }
          pcir=pcir->suc;
        }
      pcont=pcont->suc;
    }
 *zmin=zmi; *zmax=zma;
}


/*-----------------------------------------------------------------------------------------*/
int nb_trou_contour(pcont)
struct contour *pcont;
{ 
  int i;                   /* nb de trous d'un contour */
  struct circuit *pcir;
  
  if(pcont==NULL) return(0);
  i=0;
  pcir=pcont->debut_interieur;
  while(pcir) { pcir=pcir->suc; i++; }
  return(i);
}


/*___________________________________*/
int nbcontours_total(fac, nbfac)
struct modelisation_face *fac;
int nbfac;
/* Permet de recuperer le nombre de contours total 
d'un tableau de facew */
{ int comptc = 0;
  int nf;

 for(nf=0;nf<nbfac;nf++) 
	{ comptc+=nb_contour_face(fac+nf,1);
    }

 return comptc;
}

/*-----------------------------------------------------------------------------------------*/
int nb_contour_face(face,projete)
struct modelisation_face *face;
int projete;
{ 
  int i;                   /* nb de contours d'une face */
  struct contour *pcont;

  if(face==NULL)return(0);
  if(projete) pcont=face->debut_projete; else  pcont=face->debut_dessin;
  i=0;
  while(pcont) { pcont=pcont->suc; i++; }
  return(i);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
double calcul_zplan(pcir,xp,yp)
struct circuit *pcir;
double xp,yp;
{double zp,xa,ya,za;
 int i;
    /* calcul z pour axono */
 if(z_axono_pers==0)
   { zp=(pcir->vnorm[0])*xp+(pcir->vnorm[1])*yp+pcir->vnorm[3];
     zp=-zp/(pcir->vnorm[2]);
   }
    /* calcul z pour pers */
 else 
   {  zp=0.;
     denormalise_point(xp,yp,zp,&xa,&ya,&za);
     zp=(pcir->vnorm[3])/((pcir->vnorm[0])*xa+(pcir->vnorm[1])*ya-pcir->vnorm[2]);
   }
 return(zp);

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
int calculz(pcirn,pcir)
struct circuit *pcirn,*pcir;
{int i;
  for(i=0;i<4;i++)pcir->vnorm[i]=pcirn->vnorm[i];
   for(i=0;i< pcir->nbp;i++)
           pcir->z[i]=calcul_zplan(pcir,pcir->x[i],pcir->y[i]);	
 fenetre_circuit(pcir);
}
/*----------------------------------------------------------------*/
int calcul_d_du_plan(face,projete)
struct modelisation_face *face;
int projete;
{
 struct contour *pcont;
 struct circuit *pcir;

 if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
 if(pcont)	   
     { pcir=pcont->debut_support; 
       face->vnorm[3]=-(face->vnorm[0])*(pcir->x[0]);
       face->vnorm[3]-=(face->vnorm[1])*(pcir->y[0]);
       face->vnorm[3]-=(face->vnorm[2])*(pcir->z[0]);
     }
}

/*----------------------------------------------------------------*/
int coplanaire(f1,projete1,f2,projete2,epsilone,avec_meme_sens_normale,facteurd)
struct modelisation_face *f1,*f2;
int projete1,projete2;
double epsilone;
int avec_meme_sens_normale;
int facteurd;
{								// MODIFIE D.GROLEAU mars 2004
								//MODIF dec 2005

	double x1,y1,z1,x2,y2,z2,pv;

//MODIF dec 2005
	double saved1,saved2; // sauve et restore le d des plans 	//MODIF dec 2005

	saved1=f1->vnorm[3];
	saved2=f2->vnorm[3];
//MODIF dec 2005 FIN

	
	// retourne 1 si les faces sont coplanaires  a epsilone pres
	// les faces peuvent être coplanaires et avoir leur normales opposées
	// donc
	// avec_meme_sens_normale = 0 :	coplanaire quelque soit le sens de normale
	// avec_meme_sens_normale = 1 : coplanaire et meme sens de normale
	// avec_meme_sens_normale = 2 :	"coplanaire" quelque soit le sens de normale
	//                                mais sans calcul du d
	//ATTENTION, les normales sont normalisees

	// meme normale si Produit Scalaire = 1 à Epsilone près
	// meme plan    si  Delta D         = 0 à facteurd * Epsilone près

	/*
	printf("les 2 normales : \n");
	printf("%f %f %f\n",f1->vnorm[0],f1->vnorm[1],f1->vnorm[2]);
	printf("%f %f %f\n",f2->vnorm[0],f2->vnorm[1],f2->vnorm[2]);
	*/
	
	x1 = f1->vnorm[0]; y1 = f1->vnorm[1]; z1 = f1->vnorm[2];
	x2 = f2->vnorm[0]; y2 = f2->vnorm[1]; z2 = f2->vnorm[2];
	pv=x1*x2 + y1*y2 + z1*z2;

	if(avec_meme_sens_normale ==2)
	{ if(fabs(pv-1.0) <= epsilone || fabs(pv+1.0) <= epsilone)
		{  
		    //printf("(meme_sens_normale=2) coplanaire  \n");
		   return(1);
		}
	  return(0);
	}
	else
	{
	 if(fabs(pv-1.0) <= epsilone) //pv=1 normales identiques
	   { 
		 //printf("Normale identique avec face %d\n", f2->nofac_fichier);
	     calcul_d_du_plan(f1,projete1);
	     calcul_d_du_plan(f2,projete2);
		 //printf("les 2d : %f %f \n",f1->vnorm[3],f2->vnorm[3]);

		 if(fabs(f1->vnorm[3] - f2->vnorm[3]) <= facteurd*epsilone)
		 { //printf("coplanaire  %lf\n",f1->vnorm[3] - f2->vnorm[3]);
//MODIF dec 2005
			 	f1->vnorm[3]=saved1;
			 	f2->vnorm[3]=saved2;
//MODIF dec 2005 FIN
		   return(1);
		  }
	   }
	 else if(fabs(pv+1.0) <= epsilone) //pv=-1 normales opposees
	   { //printf("Normale opposee avec face %d\n", f2->nofac_fichier);
		 if(avec_meme_sens_normale == 1) return(0);
         else
		 {
          calcul_d_du_plan(f1,projete1);
	      calcul_d_du_plan(f2,projete2);
		  //printf("les 2d : %f %f \n",f1->vnorm[3],f2->vnorm[3]);

		  if(fabs(f1->vnorm[3] + f2->vnorm[3]) <= facteurd*epsilone)
		  { //printf("coplanaire  %lf\n",f1->vnorm[3] + f2->vnorm[3]);
//MODIF dec 2005
			  	f1->vnorm[3]=saved1;
			 	f2->vnorm[3]=saved2;
//MODIF dec 2005 FIN
		    return(1);
		  }
		 }
	   }
	}
//MODIF dec 2005
f1->vnorm[3]=saved1;
f2->vnorm[3]=saved2;
//MODIF dec 2005 FIN

	return(0);
}
/*----------------------------------------------------------------*/
int place_eq_plan_au_cir(face,projete)
struct modelisation_face *face;
int projete;
{
 int i;
 struct contour *pcont;
 struct circuit *pcir;
    if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
     while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<4;i++) pcir->vnorm[i]=face->vnorm[i];
	 pcir=pcont->debut_interieur;
         while(pcir)
	    { for(i=0;i<4;i++) pcir->vnorm[i]=face->vnorm[i];
	      pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
/*			VERIFICATION-LISTE			      */
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int liste_face(face,projete)
struct modelisation_face *face;
int projete;
{int i;
 struct contour *pcont;
 struct circuit *pcir;
   printf("\n\n       liste de la face no %d  \n",face->nofac_fichier);

   printf("fenetre");
   for(i=0;i<6;i++)printf(" %lf ",face->fen[i]);
   printf("\n equation du plan ");
   for(i=0;i<4;i++)printf(" %lf ",face->vnorm[i]);
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;

      while(pcont)	   
       { pcir=pcont->debut_support; 
         printf("\n fenetre support");
         for(i=0;i<6;i++)printf(" %lf ",pcir->fen[i]);
         printf("\n equation du plan ");
         for(i=0;i<4;i++)printf(" %lf ",pcir->vnorm[i]);
         printf("\n");
         for(i=0;i<pcir->nbp;i++) printf(" %lf %lf %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
	 pcir=pcont->debut_interieur;
         while(pcir)
	    { printf("\n fenetre trou");
              for(i=0;i<6;i++)printf(" %lf ",pcir->fen[i]);
              printf("\n equation du plan ");
              for(i=0;i<4;i++)printf(" %lf ",pcir->vnorm[i]);
              printf("\n");
              for(i=0;i<pcir->nbp;i++) printf(" %lf %lf %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
	      pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/

int liste_struct_contour_ecran(pcont)
struct contour *pcont;
{struct circuit *p;
 int i,j;
  if(pcont==NULL){printf("\n contour vide \n");return;}
  printf("\n\n contour pointeur = %ld suc = %ld \n",pcont,pcont->suc);
 /* if(pcont->nbcir==0)return(1); */
  printf("\n liste du support");
  p=pcont->debut_support; i=0;
  while(p)
   {liste_circuit_ecran(p);  p=p->suc; i++;}
  printf("\n liste des interieurs ");
 p=pcont->debut_interieur; i=0;
  while(p)
   {liste_circuit_ecran(p);  p=p->suc; i++;}
  return(0);

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_circuit_ecran(pcir)
struct circuit *pcir;
{int i;
  
  printf("\n\n liste du circuit pointeur = %ld suc = %ld \n",pcir,pcir->suc);

  printf("\n statut = %d ",pcir->statut);
  printf("\n fenetre ");
  for(i=0;i<6;i++)printf(" %lf ",pcir->fen[i]);
  printf("\n equation du plan ");
  for(i=0;i<4;i++)printf(" %lf ",pcir->vnorm[i]);
  printf("\n nombre de points nbp = %d \n ",pcir->nbp);
  for(i=0;i<pcir->nbp;i++)
    printf("\n x = %lf y = %lf z = %lf ",pcir->x[i],pcir->y[i],pcir->z[i]);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

/*            FENETRE ENGLOBANTE ET NORMALISATION	              */
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int fentra(pcir)
struct circuit *pcir;
{int i;
 
 for(i=0;i<3;i++)fen_aff[i]=dmin(fen_aff[i],pcir->fen[i]);
 for(i=3;i<6;i++)fen_aff[i]=dmax(fen_aff[i],pcir->fen[i]);
}

/*____________________________________________________________________*/
int cal_fen_aff()
{
    
   fen_aff[6]=dmax((fen_aff[3]-fen_aff[0]),(fen_aff[4]-fen_aff[1]));
   if(fen_aff[3]-fen_aff[0]==fen_aff[6])
	{fen_aff[1]-=(fen_aff[6]-(fen_aff[4]-fen_aff[1]))/2;}
   else
	{fen_aff[0]-=(fen_aff[6]-(fen_aff[3]-fen_aff[0]))/2;}
   fen_aff[6]=1./fen_aff[6];
   if(fen_aff[5]-fen_aff[2])fen_aff[7]=1./(fen_aff[5]-fen_aff[2]);
   else fen_aff[7]=0;
      
}
/*_______________________________________________________________________*/
int ajuste_fen_affichage(fen)
double *fen;
{              /* ajuste la fenetre globale avec fen */
               /* equivalent a fentra mais en envoyant la fenetre */
  int i;
  for(i=0;i<3;i++)fen_aff[i]=dmin(fen_aff[i],fen[i]);
  for(i=3;i<6;i++)fen_aff[i]=dmax(fen_aff[i],fen[i]);
}
/*_______________________________________________________________________*/

int normalise_face(face,projete)
struct modelisation_face *face;
int projete;
{
 struct contour *pcontour,*pcont,*pcontpred,*pcontsuc;
 struct circuit *pcir,*pcirpred,*pcirsuc;

 /* normalise une face  sur contour projete si 1 ; dessin si 0   
    normalise pts  avec sup cir
    calcul de fenetre de face
    calcul vnorm[3] du plan avec 1er contour 
    place eq plan aux circuits    
 */
 if(projete) pcontour=face->debut_projete; else pcontour=face->debut_dessin;
     pcont=pcontour; pcontpred=NULL;
     while(pcont)	   
       {pcontsuc=pcont->suc;
	pcir=pcont->debut_support; 
        normalise_cir(pcir);
	supt(pcir); 
       sup_surf_circuit(pcir);
        if(pcir->nbp>3)
          {fenetre_circuit(pcir);
	   pcir=pcont->debut_interieur; pcirpred=NULL;
           while(pcir)
	      {pcirsuc=pcir->suc;
	       normalise_cir(pcir); 
               supt(pcir);sup_surf_circuit(pcir);
               if(pcir->nbp>3){fenetre_circuit(pcir); pcirpred=pcir;}
	       else   {desalloue_circuit(pcir,3);  
	               if(pcirpred==NULL) pcont->debut_interieur=pcirsuc;
		       else pcirpred->suc=pcirsuc;
		      }
	       pcir=pcirsuc;
	      }
          pcontpred=pcont;
	  }
	 else {if(pcontpred==NULL)
                 { if(projete)face->debut_projete=pcontsuc;
                   else face->debut_dessin=pcontsuc;
                 }
	       else pcontpred->suc=pcontsuc;
               desalloue_contour(pcont,8);
	      }
	  pcont=pcontsuc; 
       } 
  /* fenetre de la face */
  fen_face(face,projete);

        /* SEULEMENT pour AXONO */
  if(z_axono_pers==0)
    { /* calcul d (vnorm[3] ) de la face */
      calcul_d_du_plan(face,projete);
    }

  /* place eq plan a chaque circuit */
     place_eq_plan_au_cir(face,projete);

}
/*-----------------------------------------------------------*/
int normalise_cir(pcir)
struct circuit *pcir;
{int j;
     /* Normalise les pts du circuit */

  for(j=0;j<pcir->nbp;j++)
     normalise_point(pcir->x[j],pcir->y[j],pcir->z[j],&pcir->x[j],&pcir->y[j],&pcir->z[j]);
	  
}

/*-----------------------------------------------------------*/
int normalise_point(xx,yy,zz,x,y,z)
double xx,yy,zz,*x,*y,*z;
{
           *x=(xx-fen_aff[0])*fen_aff[6];
	   *y=(yy-fen_aff[1])*fen_aff[6];
           if(z_axono_pers==0)*z=(zz-fen_aff[2])*fen_aff[6];
}

/*-----------------------------------------------------------*/
int denormalise_point(xx,yy,zz,x,y,z)
double xx,yy,zz,*x,*y,*z;
{
   *x=xx/fen_aff[6]+fen_aff[0];
   *y=yy/fen_aff[6]+fen_aff[1];
   if(z_axono_pers==0)*z=zz/fen_aff[6]+fen_aff[2];  
}
/*-----------------------------------------------------------------------------*/

int normalise_pts(pcir,nofac)
struct circuit *pcir;
int nofac;
{int j;
for(j=0;j<4;j++)pcir->vnorm[j]=face[nofac].vnorm[j];

for(j=0;j<pcir->nbp;j++)
	  {pcir->x[j]=(pcir->x[j]-fen_aff[0])*fen_aff[6];
	   pcir->y[j]=(pcir->y[j]-fen_aff[1])*fen_aff[6];
           pcir->z[j]=(pcir->z[j]-fen_aff[2])*fen_aff[6];
	  }

}
/*_________________________________________________________________*/
/*_________________________________________________________________*/
int denormalise_face(face,projete)
struct modelisation_face *face;
{ struct circuit *pcires,*pcir;
  struct contour *pc;
       if(projete)pc=face->debut_projete;
       else pc=face->debut_dessin;
       while(pc)
	  {denormalise_circuit(pc->debut_support);
           pcir=pc->debut_interieur;
           while(pcir)
	     { denormalise_circuit(pcir);
	       pcir=pcir->suc;
	     }
          pc=pc->suc; 
         }
}
/*_________________________________________________________________*/
/*_________________________________________________________________*/
int denormalise_circuit(pcir)
struct circuit *pcir;
{int i;
 for(i=0;i<pcir->nbp;i++)
  {pcir->x[i]=(pcir->x[i])/fen_aff[6]+fen_aff[0];
   pcir->y[i]=(pcir->y[i])/fen_aff[6]+fen_aff[1];
   pcir->z[i]=(pcir->z[i])/fen_aff[6]+fen_aff[2];
  }
}

/*____________________________________________________________________*/
/*      CALCUL ET OPERATIONS SUR LES FENETRES                         */
/*____________________________________________________________________*/

int fen_arete(p,i,f)
struct circuit *p;
int i;
struct fenetre *f;
{
          f->xmin=dmin(p->x[i],p->x[i+1]);
	  f->ymin=dmin(p->y[i],p->y[i+1]);
	  f->xmax=dmax(p->x[i],p->x[i+1]);
	  f->ymax=dmax(p->y[i],p->y[i+1]);
}
/*____________________________________________________________________*/
/*  retour = -1  les fenetres ne se coupent pas			      */
/*  retour =  1  les fenetres se coupent			      */
/*____________________________________________________________________*/
int recoup(fen1,fen2)
struct fenetre *fen1,*fen2;
{double vmax,vmin,dif;
 /*  printf("\n recoup %lf %lf %lf %lf",fen1->xmin,fen1->ymin,fen2->xmin,fen2->ymin);
   printf("\n        %lf %lf %lf %lf",fen1->xmax,fen1->ymax,fen2->xmax,fen2->ymax);*/
  vmax=dmax(fen1->xmax,fen2->xmax); vmin=dmin(fen1->xmin,fen2->xmin); 
  dif=vmax-vmin-.001;
  if((fen1->xmax-fen1->xmin)+(fen2->xmax-fen2->xmin)<dif)return(-1);
  vmax=dmax(fen1->ymax,fen2->ymax); vmin=dmin(fen1->ymin,fen2->ymin); 
  dif=vmax-vmin-.001;
 if((fen1->ymax-fen1->ymin)+(fen2->ymax-fen2->ymin)<dif)return(-1);
 return(1);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int recoup_circuit(fen1,fen2)
double *fen1,*fen2;
{double vmax,vmin,dif;
  /*printf("\n fen1 %lf %lf %lf %lf ",fen1[0],fen1[1],fen1[3],fen1[4]);*/
  /*sprintf("\n fen2 %lf %lf %lf %lf ",fen2[0],fen2[1],fen2[3],fen2[4]);*/
  vmax=dmax(fen1[3],fen2[3]); vmin=dmin(fen1[0],fen2[0]); 
  dif=vmax-vmin-.001;
  if((fen1[3]-fen1[0])+(fen2[3]-fen2[0])<dif)return(-1);
  vmax=dmax(fen1[4],fen2[4]); vmin=dmin(fen1[1],fen2[1]); 
  dif=vmax-vmin-.001;
 if((fen1[4]-fen1[1])+(fen2[4]-fen2[1])<dif)return(-1);
 return(1);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int fenetre_circuit(pcir)
struct circuit *pcir;
{int i;

 pcir->fen[0]=pcir->x[0]; pcir->fen[1]=pcir->y[0]; pcir->fen[2]=pcir->z[0];
 pcir->fen[3]=pcir->x[0]; pcir->fen[4]=pcir->y[0]; pcir->fen[5]=pcir->z[0];

 for(i=1;i<pcir->nbp;i++)
   {
    pcir->fen[0]=dmin(pcir->fen[0],pcir->x[i]);
    pcir->fen[1]=dmin(pcir->fen[1],pcir->y[i]);
    pcir->fen[2]=dmin(pcir->fen[2],pcir->z[i]);
    pcir->fen[3]=dmax(pcir->fen[3],pcir->x[i]);
    pcir->fen[4]=dmax(pcir->fen[4],pcir->y[i]);
    pcir->fen[5]=dmax(pcir->fen[5],pcir->z[i]);
   }
}
/*_______________________________________________________________________*/
int fen_face(face,projete)
struct modelisation_face *face;
int projete;
{             /* fenetre de la face sur le "projete"  si projete=1 */
  struct contour *pc;
  int i,j;

  if(projete) pc=face->debut_projete; else pc=face->debut_dessin;
  j=0;
  while(pc)
     { if(j)
         { for(i=0;i<3;i++)face->fen[i]=dmin(face->fen[i],(pc->debut_support)->fen[i]);
           for(i=3;i<6;i++)face->fen[i]=dmax(face->fen[i],(pc->debut_support)->fen[i]);
         }
       else
         { for(i=0;i<6;i++)face->fen[i]=(pc->debut_support)->fen[i];
           j=1;
         }
       pc=pc->suc;
     }
}
/*_______________________________________________________________________*/
/*                 TRANSFORMATIONS GEOMETRIQUES                          */
/*_______________________________________________________________________*/
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int tranfo()
{int i;
 double n1,n2;
 for(i=0;i<9;i++)obs.tran[i]=0;
 n1=sqrt(obs.x*obs.x+obs.y*obs.y);
 if(n1)
   {n2=sqrt(obs.x*obs.x+obs.y*obs.y+obs.z*obs.z);
    obs.tran[0]=obs.y/n1;  obs.tran[3]=obs.x/n1; obs.tran[7]=n1/n2; obs.tran[8]=obs.z/n2;}
 else {obs.tran[0]=-1; obs.tran[3]=0; obs.tran[7]=0; 
       if(obs.z<0) obs.tran[8]=-1;
       else obs.tran[8]=1;
      }

 obs.tran[1]=obs.tran[3]*obs.tran[8]; obs.tran[2]=obs.tran[3]*obs.tran[7];
 obs.tran[4]=obs.tran[0]*obs.tran[8]; obs.tran[5]=obs.tran[0]*obs.tran[7];
 obs.tran[6]=0;    obs.tran[3]=-obs.tran[3];  obs.tran[7]=-obs.tran[7];
/*
  printf(" Obs.x y z %lf %lf %lf \n",obs.x,obs.y,obs.z);
  printf(" Obs.x y z %lf %lf %lf \n",obs.xo,obs.yo,obs.zo);
  for(i=0;i<3;i++)printf(" %lf %lf %lf \n",obs.tran[i],obs.tran[i+3],obs.tran[i+6]);
*/

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int trancir(pciro,pciri,nofac)
struct circuit *pciro,*pciri;
int nofac;
{int i;
 double x,y,z;
  
 for(i=0;i<pciro->nbp;i++)
  {
   translation(pciro->x[i],pciro->y[i],pciro->z[i],&x,&y,&z);
   tranp(x,y,z,&(pciri->x[i]),&(pciri->y[i]),&(pciri->z[i]));
  }
 pciri->statut=pciro->statut;
}
/*_________________________________________________________________*/
/*_________________________________________________________________*/
int trancir_inverse(pciro,pciri)
struct circuit *pciro,*pciri;
{int i;
 double x,y,z;

 for(i=0;i<pciro->nbp;i++)
  {tranp_inverse(pciro->x[i],pciro->y[i],pciro->z[i],&x,&y,&z);
   translation_inverse(x,y,z,&(pciri->x[i]),&(pciri->y[i]),&(pciri->z[i]));
  }
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int translation(xo,yo,zo,x,y,z)
double xo,yo,zo,*x,*y,*z;
{
    *x=xo-obs.xo; *y=yo-obs.yo; *z=zo-obs.zo;
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int translation_inverse(xo,yo,zo,x,y,z)
double xo,yo,zo,*x,*y,*z;
{
    *x=xo+obs.xo; *y=yo+obs.yo; *z=zo+obs.zo;
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int tranp(xo,yo,zo,xi,yi,zi)
double xo,yo,zo,*xi,*yi,*zi;
{ 
  *xi=obs.tran[0]*xo+obs.tran[3]*yo+obs.tran[6]*zo;
  *yi=obs.tran[1]*xo+obs.tran[4]*yo+obs.tran[7]*zo;
  *zi=obs.tran[2]*xo+obs.tran[5]*yo+obs.tran[8]*zo;
  *yi=-(*yi);  *xi=-(*xi);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int tranp_inverse(xo,yo,zo,xi,yi,zi)
double xo,yo,zo,*xi,*yi,*zi;
{ 
  xo=-xo;  yo=-yo;  
  *xi=obs.tran[0]*xo+obs.tran[1]*yo+obs.tran[2]*zo;
  *yi=obs.tran[3]*xo+obs.tran[4]*yo+obs.tran[5]*zo;
  *zi=obs.tran[6]*xo+obs.tran[7]*yo+obs.tran[8]*zo;
}

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
int visible_axono(face)
struct modelisation_face *face;
{double scal;
      /* test visibilite face par Normale ;retourne 1 si VISIBLE  */
    scal=face->vnorm[0]*obs.x+face->vnorm[1]*obs.y+face->vnorm[2]*obs.z;
    if(scal>0) return(1);
    return(0);
}

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
int tran_face(face,proj,fact,projt)
struct modelisation_face *face,*fact;
int projt,proj;
{struct contour *pcont,*pct,*pc,*pctdeb;
 struct circuit *pcir,*pcour,*pciri;
 int i;
 if(fact==NULL)
       {printf("\n pb tran_face fact non allouee \n"); exit(0);}

  if(proj)pcont=face->debut_projete;
  else pcont=face->debut_dessin;

  pct=NULL; pctdeb=NULL;
  while(pcont)
    {pc=alloue_contour(345);
     for(i=0;i<nb_etat;i++)pc->etat[i]=pcont->etat[i];
     if(pct==NULL){pct=pc; pctdeb=pc;}
     else {pct->suc=pc; pct=pc;}
     pciri=alloue_circuit(50);
    
 if(pciri==NULL)pb_allocation(50); pciri->suc=NULL;
     pcir=pcont->debut_support; 
     pciri->nbp=pcir->nbp;  alloue_point_circuit(pciri,51);
     trancir(pcir,pciri,0);
     fenetre_circuit(pciri);  sensz(pciri);
     pc->debut_support=pciri;
     fentra(pciri);  
     pcir=pcont->debut_interieur;
     while(pcir)
        {pciri=alloue_circuit(50);
         if(pciri==NULL)pb_allocation(50); pciri->suc=NULL;
         pciri->nbp=pcir->nbp;   alloue_point_circuit(pciri,52);
         trancir(pcir,pciri,0); pciri->suc=NULL; 
         sensz(pciri);
         if(pc->debut_interieur==NULL)pc->debut_interieur=pciri; 
         else pcour->suc=pciri;
         pcour=pciri;
         pcir=pcir->suc;
       }
     pcont=pcont->suc;
    }
   if(projt)
	{desalloue_chaine_contour(fact->debut_projete,1044);
	 fact->debut_projete=pctdeb;
	}
   else 
	{desalloue_chaine_contour(fact->debut_dessin,1045);
	 fact->debut_dessin=pctdeb;
	}

  fen_face(fact,projt);
 

 }
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
int tran_face_inverse(face,proj)
struct modelisation_face *face;
int proj;
{struct contour *pcont,*pct,*pc,*pctdeb;
 struct circuit *pcir,*pcour,*pciri;
 int i;
  if(proj)pcont=face->debut_projete;
  else pcont=face->debut_dessin;
  if(pcont==NULL)return;
  pct=NULL;
  while(pcont)
    {pc=alloue_contour(345); 
     for(i=0;i<nb_etat;i++)pc->etat[i]=pcont->etat[i];
     if(pct==NULL){pct=pc; pctdeb=pc;}
     else {pct->suc=pc; pct=pc;}
     pciri=alloue_circuit(50); if(pciri==NULL)pb_allocation(50);
     pciri->suc=NULL;
     pcir=pcont->debut_support; 
     pciri->nbp=pcir->nbp;  alloue_point_circuit(pciri,51);
     trancir_inverse(pcir,pciri); fenetre_circuit(pciri);
     pc->debut_support=pciri;
     pcir=pcont->debut_interieur;
     while(pcir)
        {pciri=alloue_circuit(50); if(pciri==NULL)pb_allocation(50);
         pciri->suc=NULL;
         pciri->nbp=pcir->nbp;   alloue_point_circuit(pciri,52);
	 trancir_inverse(pcir,pciri); fenetre_circuit(pciri);
         pciri->suc=NULL; 
        if(pc->debut_interieur==NULL)pc->debut_interieur=pciri; 
         else pcour->suc=pciri;
         pcour=pciri;
         pcir=pcir->suc;
       }
     pcont=pcont->suc;
    }

  if(proj)
     {desalloue_chaine_contour(face->debut_projete,1046);
      face->debut_projete=pctdeb;
     }
  else 
     {desalloue_chaine_contour(face->debut_dessin,1047);
      face->debut_dessin=pctdeb;
     }  
  fen_face(face,proj);
}


/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
int tran_normale_inverse(norm)
double *norm;
{ double xo,yo,zo;
   tranp_inverse(norm[0],norm[1],norm[2],&xo,&yo,&zo);
   norm[0]=xo; norm[1]=yo; norm[2]=zo;

}
/*-----------------------------------------------------------------------------*/
int tran_normale(norm)
double *norm;
{ double xo,yo,zo;
   tranp(norm[0],norm[1],norm[2],&xo,&yo,&zo);
   norm[0]=xo; norm[1]=yo; norm[2]=zo;
}
/*-----------------------------------------------------------*/
int test_si_inv_sens(face,projete)
struct modelisation_face *face;
int projete;
{int i;
 struct contour *pcont;
 struct circuit *pcir;
 double nor[4];
 
 /* test si sens circuit donne sens de la normale */
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
   if(pcont)
      { pcir=pcont->debut_support; 
        normale(pcir,nor);
        if((face->vnorm[0]*nor[0]+face->vnorm[1]*nor[1]+face->vnorm[2]*nor[2])<0)             { inverse_face(face,projete);
            }
      }
}
/*-----------------------------------------------------------*/
int test_sens(face,projete)
struct modelisation_face *face;
int projete;
{int i,sens;
 struct contour *pcont;
 struct circuit *pcir;
 double nor[4];
 sens=0;
 /* test si sens circuit donne sens de la normale */
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
    while(pcont)
      { pcir=pcont->debut_support; 
        while(pcir)
         {if(surface(pcir->nbp,pcir->x,pcir->y)>0)
	    { /*printf("\n face support %d dans le mauvais sens\n",face->nofac_fichier);
	     liste_circuit_ecran(pcir); */
             sens=1;
            }
           pcir=pcir->suc;
         }
        pcont=pcont->suc;
      }
return(sens);
}

/*-----------------------------------------------------------*/
int inverse_face(face,projete)
 struct modelisation_face *face;
int projete;
{int i;
 struct contour *pcont;
 struct circuit *pcir;
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
  while(pcont)	   
       { pcir=pcont->debut_support;
         invsens(pcir);
         pcir=pcont->debut_interieur;
         while(pcir)
	    { invsens(pcir);
	      pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}

/*____________________________________________________________________*/
int trancirsol(pciro,pciri,nsol)
struct circuit *pciro,*pciri;
double *nsol;
{int i;
  for(i=0;i<pciri->nbp;i++)
     {pciri->x[i]=pciro->x[i]; pciri->y[i]=pciro->y[i];
      pciri->z[i]=-(nsol[0]*pciro->x[i]+nsol[1]*pciro->y[i]+nsol[3])/nsol[2];
     }
}
/*-----------------------------------------------------------*/
int pers_conic_face(face,projete)
struct modelisation_face *face;
int projete;
{
 struct contour *pcont;
 struct circuit *pcir;
 int i;
/*
printf("face no %d\n",face->nofac_fichier); 
liste_face(face,projete);
*/
      /* transforme en pers avec reajuste fenetre affichage */
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
   while(pcont)    
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
            { pcir->x[i]=-pcir->x[i]/pcir->z[i];
              pcir->y[i]=-pcir->y[i]/pcir->z[i];
/*printf("point pers %lf %lf\n",pcir->x[i],pcir->y[i]);*/
            }
         fenetre_circuit(pcir); 
         pcir=pcont->debut_interieur;
         while(pcir)
            { for(i=0;i<pcir->nbp;i++) 
                { pcir->x[i]=-pcir->x[i]/pcir->z[i];
                  pcir->y[i]=-pcir->y[i]/pcir->z[i];
                }
              fenetre_circuit(pcir);
              pcir=pcir->suc;
            }
         pcont=pcont->suc; 
       } 
   fen_face(face,projete);
   ajuste_fen_affichage(face->fen);
}
/*-----------------------------------------------------------*/
/*							     */
/*             SPHERE ET ELLIPSE		             */
/*							     */
/*-----------------------------------------------------------*/

int proj_sphere_face(face,projete)
struct modelisation_face *face;
int projete;
{
 struct contour *pcont;
 struct circuit *pcir;
 double dist,x,y,z;
 int i;
       /* transforme en projection sherique avec reajuste fenetre affichage */
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
   while(pcont)    
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
            { x=pcir->x[i];
              y=pcir->y[i];
              z=pcir->z[i];
              dist=sqrt(x*x+y*y+z*z);
              pcir->x[i]=x/dist;
              pcir->y[i]=y/dist;
              pcir->z[i]=z/dist;
            }
         fenetre_circuit(pcir); 
         pcir=pcont->debut_interieur;
         while(pcir)
            { for(i=0;i<pcir->nbp;i++) 
                { x=pcir->x[i];
                  y=pcir->y[i];
                  z=pcir->z[i];
                  dist=sqrt(x*x+y*y+z*z);
                  pcir->x[i]=x/dist;
                  pcir->y[i]=y/dist;
                  pcir->z[i]=z/dist;
                }
              fenetre_circuit(pcir);
              pcir=pcir->suc;
            }
         pcont=pcont->suc; 
       } 
   fen_face(face,projete);
   ajuste_fen_affichage(face->fen);
}


/*-----------------------------------------------------------*/
int proj_nusselt(pcir,pcir_temp)
struct circuit *pcir,*pcir_temp;
{
  int i;
  double petit_axe,grand_axe,gamma,ga,gb;
  double poab[4],pxoy[4],xt[3],yt[3],zt[3];
char *out;

  grand_axe=1.;
  pxoy[0]=0; pxoy[1]=0; pxoy[2]=-1; pxoy[3]=0;
  poab[3]=0; xt[0]=0; yt[0]=0; zt[0]=0;
  xt[2]=pcir->x[0];
  yt[2]=pcir->y[0];
  zt[2]=pcir->z[0];
  pcir_temp->x[pcir_temp->nbp]=xt[2];
  pcir_temp->y[pcir_temp->nbp]=yt[2];
  pcir_temp->z[pcir_temp->nbp]=zt[2];
  pcir_temp->nbp=pcir_temp->nbp+1;

 for(i=0;i<pcir->nbp-1;i++)
    { /* traite une arete ab (i,i+1) */
       xt[1]=xt[2]; yt[1]=yt[2]; zt[1]=zt[2]; 
       xt[2]=pcir->x[i+1];
       yt[2]=pcir->y[i+1];
       zt[2]=pcir->z[i+1];
      /* normale du plan oab */       
       /*ieee_flags("clear","exception","all",&out);*/
       normale_avec_3pts(xt,yt,zt,poab);
       /*ieee_flags("get","exception","overflow",&out);*/
       if(fabs(poab[2])>0.001 && out[2]!='v')
          { /* projection suivant arc ellipse  si petit axe > 0.001*/
            /* angle alpha (plan oab et plan xoy) ; petit axe = cos(alpha) */
              petit_axe=fabs(poab[2]);
            /* angle gamma entre droite inter (plan oab et plan xoy) et axe ox */
              if(poab[1]) gamma=atan(-poab[0]/poab[1]);
              else gamma=pi/2;
/*
printf("arete %d : %lf %lf %lf %lf\n",i,xt[1],yt[1],xt[2],yt[2]);
printf("   ellipse  petit axe %lf gamma %lf\n",petit_axe,gamma*180/pi);
*/
            /* definit equation de l'ellipse */
              def_ellipse(grand_axe,petit_axe,gamma,pcir_temp);
              poly_arc_ellipse(xt[1],yt[1],xt[2],yt[2],pcir_temp);
          }
       else
          { /* projection suivant segment ,peut_etre un point */
/*printf("arete %d : %lf %lf %lf %lf\n",i,xt[1],yt[1],xt[2],yt[2]);*/
             pcir_temp->x[pcir_temp->nbp]=xt[2];
             pcir_temp->y[pcir_temp->nbp]=yt[2];
             pcir_temp->z[pcir_temp->nbp]=zt[2];
             pcir_temp->nbp=pcir_temp->nbp+1;
          }
    }
}

/*---------------------------------------------------*/
int def_ellipse(a,b,alpha)
double a,b,alpha;
{
 double cosalpha,sinalpha;
 double ca,cb,cc;

  cosalpha=cos(alpha); sinalpha=sin(alpha);

  /*****************************************************/
  /* equation de l'ellipse en coordonnees cartesiennes */
  /*             ca*y2+cb*xy+cc*x2=1                   */
  /*****************************************************/
  ca=(cosalpha*cosalpha)/(b*b)+(sinalpha*sinalpha)/(a*a);
  cb=2*cosalpha*sinalpha*(1/(a*a)-1/(b*b));
  cc=(cosalpha*cosalpha)/(a*a)+(sinalpha*sinalpha)/(b*b);

  /******************************************************/
  /* equation de l'ellipse en coordonnees parametriques */
  /*    x=pax*cos(teta)+pbx*sin(teta)                     */
  /*    y=pay*cos(teta)+pby*sin(teta)                     */
  /*           0<teta<2*PI		                */
  /******************************************************/
    pax=a*cosalpha;  pbx=-b*sinalpha;
    pay=a*sinalpha;  pby=b*cosalpha;
}

/*---------------------------------------------------*/
int poly_arc_ellipse(x1,y1,x2,y2,pcir)
double x1,x2,y1,y2;
struct circuit *pcir;  
{      /* genere arc ellipse par segmentation moins le 1er pt */
 double teta1,x1d,x2d,teta;
 int nbp,sens;

   nbp=pcir->nbp;

   x1d=ang_ellipse(x1,y1);
   x2d=ang_ellipse(x2,y2);
/*printf(" angle avec ellipse %lf %lf\n",x1d,x2d);*/

   if(fabs(x2d-x1d)<180)
      { if((x1d-x2d)<0) sens=1; 
        else sens=0;
      } 
   else
      { if((x1d-x2d)<0) { x2d=x2d+360; sens=1; }
        else { x1d=x1d+360; sens=0; }
      }

   if(sens)
      { teta=x1d+2; 
        while(teta<x2d)
          { teta1=teta*pi/180.;
            pcir->x[nbp]=pax*cos(teta1)+pbx*sin(teta1); 
            pcir->y[nbp]=pay*cos(teta1)+pby*sin(teta1);
            nbp++; 
            teta=teta+2.;
          }
      }
   else
      { teta=x1d-2; 
        while(teta>x2d)
          { teta1=teta*pi/180.;
            pcir->x[nbp]=pax*cos(teta1)+pbx*sin(teta1); 
            pcir->y[nbp]=pay*cos(teta1)+pby*sin(teta1);
            nbp++; 
            teta=teta-2.;
          }
      }
   
   pcir->x[nbp]=x2; 
   pcir->y[nbp]=y2;
   nbp++;
 
   pcir->nbp=nbp;
}

/*---------------------------------------------------*/
double ang_ellipse(x,y)
double x,y;
{double xd,tg,si,teta;
          /* renvoie angle en degre */
  tg=(pax*y-pay*x)/(pby*x-pbx*y);
  si=(pax*y-pay*x)/(pax*pby-pbx*pay);
 
  if(tg*si>0)teta=atan(tg);
  else if(tg>0 && si<0)teta=atan(tg)+pi;
  else if(tg<0 && si>0)teta=pi+atan(tg);

  xd=teta*180./pi;
  if(xd<0)xd=360+xd;
  return(xd);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int verification_des_pointeurs(face)
struct modelisation_face *face;
{int i,j;
 struct contour *pcont;
 struct circuit *pcir;
struct modelisation_face *ff;

/* 
if(face==NULL)return(0);

      pcont=face->debut_projete;
      while(pcont)	   
       { 
	 i=verif_contour(pcont,2);
		 pteur_contour[i]=-pteur_contour[i];
	 pcir=pcont->debut_support;
	 while(pcir)
	   {i=verif_circuit(pcir,2); 
		pteur_circuit[i]=-pteur_circuit[i];
	    i=verif_circuit_point(pcir->x,2); 
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->y,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->z,2);
			 pteur_circuit_point[i]=-pteur_circuit_point[i];
	    pcir=pcir->suc;
	   }
	 pcir=pcont->debut_interieur;
	 while(pcir)
	   {i=verif_circuit(pcir,2); 
		pteur_circuit[i]=-pteur_circuit[i];
	    i=verif_circuit_point(pcir->x,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->y,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->z,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    pcir=pcir->suc;
	   }
         pcont=pcont->suc; 
       } 

      pcont=face->debut_dessin;
      while(pcont)	   
       { 
	 i=verif_contour(pcont,2);
		 pteur_contour[i]=-pteur_contour[i];
	 pcir=pcont->debut_support;
	 while(pcir)
	   {i=verif_circuit(pcir,2); 
			pteur_circuit[i]=-pteur_circuit[i];
	    i=verif_circuit_point(pcir->x,2); 
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->y,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->z,2);
			 pteur_circuit_point[i]=-pteur_circuit_point[i];
	    pcir=pcir->suc;
	   }
	 pcir=pcont->debut_interieur;
	 while(pcir)
	   {i=verif_circuit(pcir,2); 
			pteur_circuit[i]=-pteur_circuit[i];
	    i=verif_circuit_point(pcir->x,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->y,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    i=verif_circuit_point(pcir->z,2);
			pteur_circuit_point[i]=-pteur_circuit_point[i];
	    pcir=pcir->suc;
	   }
         pcont=pcont->suc; 
       } 
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int liste_verification_des_pointeurs()
{

/* 
liste_pointeur_contour();
liste_pointeur_circuit();
liste_pointeur_circuit_point();
liste_pointeur_point();
liste_pointeur_poly();
liste_pointeur_liste_point();
liste_pointeur_inters();

remet_verif_pteur_solene();
  */

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_circuit()
{int i,j;
 struct circuit *pt;

/* 
printf("\n\n liste des pointeurs circuit \n");

 for(i=0;i<NBCIRMAX;i++)
  {if(pteur_circuit[i]>0)
      {printf("\n i = %d pteur = %ld ",i,pteur_circuit[i]);
	pt=pteur_circuit[i];
       printf("\n      point %ld %ld %ld ",pt->x,pt->y,pt->z);
      }
  }
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_poly()
{int i,j;
 struct circuit *pcir;

/* 
printf("\n\n liste des pointeurs poly \n ");

 for(i=0;i<NBPOLYMAX;i++)
  {if(pteur_poly[i]>0)
    { printf("\n i = %d pteur = %ld ",i,pteur_poly[i]);}
  }
  */
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_contour()
{int i,j;
 struct circuit *pcir;
 struct contour *pt;

/* 
printf("\n\n liste des pointeurs contour \n");

 for(i=0;i<NBCONTMAX;i++)
  {if(pteur_contour[i]>0)
    {printf("\n i = %d pteur = %ld ",i,pteur_contour[i]);
     pt=pteur_contour[i];
     pcir=pt->debut_support;
     while(pcir)
        {printf("\n circuit support %ld",pcir);pcir=pcir->suc;}
     pcir=pt->debut_interieur;
     while(pcir)
        {printf("\n circuit interieur %ld",pcir);pcir=pcir->suc;}
    }
  }
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_circuit_point()
{int i;
/* 
printf("\n\n liste des pointeurs circuit_point \n");

 for(i=0;i<NBPTCIRMAX;i++)
  {if(pteur_circuit_point[i]>0)
     {printf("\n i = %d pteur = %ld ",i,pteur_circuit_point[i]);}
  }
  */
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_point()
{int i;

/* 
printf("\n\n liste des pointeurs point \n");

 for(i=0;i<NBPOINT;i++)
  {if(pteur_point[i]>0)
    {
     printf("\n i = %d pteur = %ld ",i,pteur_point[i]);
    }
  }
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_face()
{int i;

/* 
printf("\n liste des pointeurs face \n");

 for(i=0;i<NBFACEMAX;i++)
  {if(pteur_face[i]>0)
    {
     printf("\n i = %d pteur = %ld ",i,pteur_face[i]);
    }
  }
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_inters()
{int i;

/* 
printf("\n liste des pointeurs inters \n");

 for(i=0;i<NBINTERS;i++)
  {if(pteur_inters[i]>0)
    {
     printf("\n i = %d pteur = %ld ",i,pteur_inters[i]);
    }
  }
  */
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int liste_pointeur_liste_point()
{int i;

/* 
printf("\n liste des pointeurs liste_point \n");

 for(i=0;i<NBLISTEPOINT;i++)
  {if(pteur_liste_point[i]>0)
    {
     printf("\n i = %d pteur = %ld ",i,pteur_liste_point[i]);
    }
  }
  */
}


/*____________________________________________________________________*/

remet_verif_pteur_solene()
{int i;

/* 
  for(i=0;i<NBCIRMAX;i++)
    {if(pteur_circuit[i]<0)pteur_circuit[i]=-pteur_circuit[i];}

  for(i=0;i<NBCONTMAX;i++)
    {if(pteur_contour[i]<0)pteur_contour[i]=-pteur_contour[i];}

  for(i=0;i<NBPTCIRMAX;i++)
    {if(pteur_circuit_point[i]<0)
		pteur_circuit_point[i]=-pteur_circuit_point[i];}
  */

}
/*____________________________________________________________________*/
int liste_des_pointeurs()
{
liste_pointeur_face();
liste_pointeur_contour();
liste_pointeur_circuit();
liste_pointeur_circuit_point();
liste_pointeur_point();
liste_pointeur_poly();
liste_pointeur_liste_point();
liste_pointeur_inters();
}


/*____________________________________________________________________*/
/*            TRAITE VU/CACHE pour faces coplanaires		          */
/*____________________________________________________________________*/
int traite_coplanerite(x_obs,y_obs,z_obs,fac_obs,fac_masc,nbfac_masc,avec_meme_normale)
double x_obs,y_obs,z_obs;
struct modelisation_face *fac_obs, *fac_masc;
int nbfac_masc;
int avec_meme_normale;
{
    int i,oui;
	double x_sav,y_sav,z_sav,xp,yp,zp;
	double x_sav1,y_sav1,z_sav1, tran_sav[9];
	struct modelisation_face *fac_temp;

	// le coplanerite peut tenir compte du sens de la normale si avec_meme_normale =1

// test si le point obs de la face obs,
// coplanaire avec une des  nbfac_masc fac_masc du masque
// est à l'intérieur du masque (de la face coplanaire)
// si oui return 1, l'observateur est masqué

	// PB : le point en traitement peut être ds le plan coplanaire avec lui-même
	// En supposant qu'on trouve toujours dans le masque la face en traitement
	// alors, il faut trouver au moins 2 faces coplanaires

// sauve l'observateur et la tranfo
	x_sav = obs.x; y_sav = obs.y; z_sav = obs.z; 
	x_sav1 = obs.xo; y_sav1 = obs.yo; z_sav1 = obs.zo; 
	for (i=0;i<9;i++) tran_sav[i] = obs.tran[i];
	
// calcule la transfo pour traiter le pb en 2D
	obs.x=fac_obs->vnorm[0];
	obs.y=fac_obs->vnorm[1];
	obs.z=fac_obs->vnorm[2];

    obs.xo = 0; //translation nulle
    obs.yo = 0;
    obs.zo = 0;

    tranfo();  

	tranp(x_obs,y_obs,z_obs,&xp,&yp,&zp);
    //printf("xp,yp, %f %f\n",xp,yp);

// cherche face de masque coplanaire avec face en traitement
	oui=0;
	for (i=0;i<nbfac_masc;i++)
	{ if(coplanaire(fac_obs,1,fac_masc+i,1,0.00005,avec_meme_normale) == 1)
		   { //printf("coplanaire avec face %d \n",(fac_masc+i)->nofac_fichier);

		     fac_temp=alloue_face(1,634);
             tran_face(fac_masc+i,1,fac_temp,1);

			 //test si observateur dans masque
			 if (point_dans_face(xp,yp,fac_temp,1))
			   { //printf("dedans\n");
				 oui++;
				 if(oui==2)
				 {
				  desalloue_face(fac_temp);
			      // restaure l'observateur
	              obs.x = x_sav; obs.y = y_sav; obs.z = z_sav; 
	              obs.xo = x_sav1; obs.yo = y_sav1; obs.zo = z_sav1; 
				  for (i=0;i<9;i++)  obs.tran[i] = tran_sav[i];

			      return(1);
				 }
			   }
			 desalloue_face(fac_temp);
		   }

	}

// restaure l'observateur
	obs.x = x_sav; obs.y = y_sav; obs.z = z_sav; 
	obs.xo = x_sav1; obs.yo = y_sav1; obs.zo = z_sav1; 
	for (i=0;i<9;i++)  obs.tran[i] = tran_sav[i];


	return(0);
}


/*____________________________________________________________________*/
/*    tableau de VISIBILITE (observateur, scene triangulee)            */
/*____________________________________________________________________*/

/*_____________________________________________________________________*/
/* Lecture des facteurs de visibilite dans fichier */

void read_ligne_vis(pff, ligne_ff, nb_contour, i)
FILE *pff;
float *ligne_ff;
int nb_contour;
int i; /* Ligne */

/* Lit une ligne specifiee dans un fichier de facteurs de visibilité*/
/* arranges en Fij , i variant de 0 a nb_contour-1 */
{
	//int k;
	//printf("nbcontour, noc %d %d\n",nb_contour, i);
    fseek(pff,(i*nb_contour)*sizeof(float),SEEK_SET);
    fread(ligne_ff, sizeof(float), nb_contour, pff);

	//for(k=0;k<nb_contour;k++) printf("vis %f",ligne_ff[k]);
}
