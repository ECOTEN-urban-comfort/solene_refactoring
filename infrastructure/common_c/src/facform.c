/* facformn.c : DETERMINATION DES FACTEURS DE FORME DE CONTOUR A CONTOUR */
/* Pour triangles et quadrangles 					 */
/* Modifs MJ Antoine Janv./fev. 1998 ,
   Modif   D.Groleau  Octobre 1998	
					  Juillet 2001 (angle de vue en paramètre)		 */

// D. GROLEAU  mars 2004  // modifie COPLANAIRE
// D. GROLEAU  juillet 2005 // écriture sur disque en bloc des fac forme par rapport à une face
/* l'executable est stocké en facform.exe et utilisé en standard par Solene */




/*______________________________________________________________________ */

/*
cc facformn.c pers_util.o solutile.o face_op_face.o lib_solene_94.o geomutile.o poly_op_poly.o traite.o solaire.o -o facformn -lm
*/

/*______________________________________________________________________*/
/* POUR UN FICHIER .cir 						*/
/* determine les facteurs de forme entre tous les contours du fichier_1,*/
/* en tenant compte des masques du fichier_2 				*/
/* le fichier_1 etant la triangulation de tout ou partie du fichier_2 	*/
/*______________________________________________________________________*/

// ATTENTION  modif de mars 2004:
// Ne tient pas compte des faces situées dans le même plan que celui du contour de la face en traitement


#include<solene.h>

/* Declarations de structures */
struct vertex { double x, y, z; };
struct vector { double cx, cy, cz; };
struct triangle { struct vertex a, b, c; };


/* Fonctions */
extern int option_calcul_z;   /* option CALCUL du Z ds FACE-OP-FACE 1=oui */
                              /* utilise ds singul.c epure_polygone */
			      /* et dans face_op_face.c             */
extern 	double coef_discol;
extern 	int z_axono_pers;

/*** Ajoutees par M.J.A. 02-98 ***/
struct circuit* alloue_cir();
void desalloue_cir();
void affiche_tri();
void ne_voit_rien();
double norme_vect3D();
void format_facform();
void prod_vect_3D();
double prod_scal_3D();
int sinus_vect3D();
int cosinus_vect3D();
double surface_tri();
double distance_entre_patchs();
double longueur_arete_max();
double calcul_ff(); 
double  traite_triangle();
double  traite_quadrangle();
void divise_triangle();
void traitement_faces();
/*** ***/
/*** Modifiees par M.J.A. ***/
/* void evalue_ff() */
void preevalue_ff_visib();
/* void fait_pers_fac1() */
void fait_pers_fac_visib();
void ff_des_patch_ds_face();
double 	fform_yamanouti();

/*** ***/
/* Autres fonctions preexistantes */

struct vertex average();
double longueur_arete_poly();


/* Variables globales */
/* n'ayant pu etre supprimees: */

double 	ang,angvis,pi,tgvis,covis,sinvis;
double vnf[3];
FILE *pform,*pval,*pval1;
int sortie;
double somme_fac_form,epsilone;

 double	 epsiloneN;  // parametre de coplanaire
 int	 facteurD;   // parametre de coplanaire



/*_________________________________________________________________*/
main(argc,argv)           /* Facteur Forme pour un FICHIER DE FACES */
int argc;char **argv;
	{

/* Fichiers et structures de donnees */
	char nom_in[256],nom_masc[256],nom_out_ff[256];
	int nbfac,nbfac_masque;
	struct modelisation_face *fac,*fac_visib,*fac_masque;
 	FILE *pfic,*pfic1;
	char *s_dir;
	int nomax, nomaxm;
	double englob[10];
/* Autres */
	int im = 0;
 	double ang;

printf("\nProgramme facformn\n\n");	

/* initialisation */

   	singularite=0; non_singularite=0; nb_etat=0;
   	pb_masque=0; colle_max=coef_discol*DISCOL;
   	pi=4*atan(1.);

   // initialisation pour coplanérité
   epsiloneN = 0.0001;
   facteurD= 1000;

/* champ de vision : 1/2 angle au sommet */
   	ang=89.9;
   	angvis=pi*ang/180.;
   	tgvis=tan(angvis); covis=cos(angvis); sinvis=sin(angvis);


/* lecture parametres commande */
	s_dir = (char *)getenv("PWD");  

   	if(argc<7) format_facform();

	epsilone = 0.001; // pour planimétrie des faces
	if(argc==8)	
	{ sscanf(argv[7],"%lf",&epsilone);
	}
	printf("\nepsilone de planimétrie des faces %f\n",epsilone); 
	if(argc==9)	
	{ sscanf(argv[8],"%lf",&ang);
	  angvis=pi*ang/180.;
   	  tgvis=tan(angvis); covis=cos(angvis); sinvis=sin(angvis);
	}
	printf("\n1/2 angle de vision :  %f\n",ang); 

 
        compose_nom_complet(nom_in,s_dir,argv[1],"cir");  
        printf("\n\nfichier a traiter : %s \n",nom_in);

        compose_nom_complet(nom_masc,s_dir,argv[2],"cir");  
        printf("avec fichier masque: %s \n",nom_masc);

	    sscanf(argv[3],"%d",&sortie);

        compose_nom_complet(nom_out_ff,s_dir,argv[4],"fac");  
        printf("fichier facteurs de forme (.fac) : %s \n",nom_out_ff);

/* ouvre les fichiers a traiter et ecrit en tete fichier resultat */

       if((pfic1=fopen(nom_in,"r"))==NULL)
		{
         printf("\n impossible ouvrir %s\n",nom_in); 
		 exit(0);
		}
            
       if((pfic=fopen(nom_masc,"r"))==NULL)
		{
         printf("\n impossible ouvrir %s\n",nom_masc); 
		 exit(0);
  		}
          
       if((pform=fopen(nom_out_ff,"wb"))==NULL)
		{
         printf("\n impossible ouvrir %s\n",nom_out_ff); 
		 exit(0);
		}

/* lecture fichier a traiter */
       lit_en_tete(pfic1,&nbfac,&nomax,englob);
       printf("\nTraite %d faces ",nbfac);
       fac=alloue_face(nbfac,34);
       lit_fic_cir3d(pfic1,nbfac,fac);
       fclose(pfic1);

/* lecture  fichier masque */
       lit_en_tete(pfic,&nbfac_masque,&nomaxm,englob);
       printf("avec %d faces masque \n\n",nbfac_masque);
       fac_visib=alloue_face(nbfac_masque,34);
       fac_masque=alloue_face(nbfac_masque,34);
       lit_fic_cir3d(pfic,nbfac_masque,fac_masque);
       fclose(pfic);

/** ECRITURE Des FICHIERS .val **/
      compose_nom_complet(nom_out_ff,s_dir,argv[5],"val");  
      printf("descripteur a creer : %s \n",nom_out_ff);

	  if ((pval = fopen(nom_out_ff,"w")) == NULL)
	   {
		printf("\n  impossible de creer %s !!\n\n", nom_out_ff);
		exit(0);
	   }
	  fprintf (pval,"%d %d %8.5f %8.5f\n", nbfac, nomax, 0., 1.);   

      compose_nom_complet(nom_out_ff,s_dir,argv[6],"val");  
      printf("descripteur a creer : %s \n",nom_out_ff);

	  if ((pval1 = fopen(nom_out_ff,"w")) == NULL)
	   {
		printf("\n  impossible de creer %s !!\n\n", nom_out_ff);
		exit(0);
	   }
	  fprintf (pval1,"%d %d %8.5f %8.5f\n", nbfac, nomax, 0., 1.);   

/* lancement du calcul */
       	printf("Evalue les facteurs ...\n");

        traitement_faces(nbfac, fac, nbfac_masque, fac_masque, fac_visib);


       	desalloue_fface(fac,nbfac);
       	desalloue_fface(fac_visib,nbfac_masque);
       	desalloue_fface(fac_masque,nbfac_masque);
       	fclose(pform);
		fclose(pval); fclose(pval1);


	creer_OK_Solene();
	printf("\nFin calcul facformn\n\n");	

	exit(0);
}


/*___________________________________________________________________________________*/
void traitement_faces(nbfac, fac, nbfac_masque, fac_masque, fac_visib)
int nbfac;
struct modelisation_face *fac;
int nbfac_masque;
struct modelisation_face *fac_masque;
struct modelisation_face *fac_visib;

{ /* Le resultat du calcul des facteurs de forme (lance par la procedure preevalue_ff_visib() */
  /* se trouvera dans le fichier resultat (il n'y a pas de structure maintenant les facteurs  */
  /* de forme en memoire */
int nf;
int noc;
int comptc = 0;
double normal_x,normal_y,normal_z;
struct contour *pcont;
struct circuit *cir_emetteur;
struct vector normale_emetteur;
int nof_max_somme,noc_max_somme;
double max_somme_fac;

max_somme_fac =0; nof_max_somme = 0; noc_max_somme = 0;

for(nf=0;nf<nbfac;nf++) 
  { 		
	 printf("Face numero %d\n", (fac+nf)->nofac_fichier);
        normal_x=(fac+nf)->vnorm[0];
	    normal_y=(fac+nf)->vnorm[1];
	    normal_z=(fac+nf)->vnorm[2];

   		vnf[0]=normal_x;
		vnf[1]=normal_y; 
		vnf[2]=normal_z;

		normale_emetteur.cx=normal_x;
		normale_emetteur.cy=normal_y;
		normale_emetteur.cz=normal_z;

   

		/* observateur regarde dans le sens de la normale a la face */
        obs.x=-normal_x;
        obs.y=-normal_y;
        obs.z=-normal_z;

		/* transformation a appliquer a chaque contour de la face */
        tranfo();

		/* ecrit .val pour la face */
		fprintf (pval,"f%d %d\n",(fac+nf)->nofac_fichier,nb_contour_face(fac+nf,1));   
        fprintf (pval1,"f%d %d\n",(fac+nf)->nofac_fichier,nb_contour_face(fac+nf,1));   

		noc=0;
		pcont=(fac+nf)->debut_projete;

        while(pcont) 	/* Balayage des contours de la face courante */   
          { 
            obs.xo=0; obs.yo=0; obs.zo=0; 
			
		 	cir_emetteur=pcont->debut_support;   /* Pointeur sur le contour courant */
            centre_de_gravite(cir_emetteur, &obs.xo, &obs.yo, &obs.zo);
 		 	noc++; 
			comptc++;

			//printf(" Contour emetteur no %d >>>> indice contour interne %d\n", noc,comptc); 

			if(sortie) fprintf(pform,"face %d contour %d\n",(fac+nf)->nofac_fichier,noc);

			//attention, si modif remettre le dernier paramètre manquant dans traite coplanérite () voir solutile.c

			// if(traite_coplanerite(obs.xo,obs.yo,obs.zo,fac+nf,fac_masque,nbfac_masque,memeSENSDENORMALE))
			/* EN COMMENTAIRE
			if(traite_coplanerite(obs.xo,obs.yo,obs.zo,fac+nf,fac_masque,nbfac_masque))
			     {  
					//l'observateur est masqué totalement
					printf("   CACHE (face %d contour %d)\n",(fac+nf)->nofac_fichier,noc);
			        ne_voit_rien(nbfac,fac);
		         }
		      else */
		         {
				 
					/* Visibilite en tenant compte des masques */
					/* et calcul des facteurs de forme du contour courant emetteur */
					/* vers les contours des autres faces du fichier */

			
					fait_pers_fac_visib(nbfac_masque, fac_masque, fac_visib, (fac+nf));

					somme_fac_form =0;

					preevalue_ff_visib(nbfac, fac, nbfac_masque, fac_visib, fac_masque, (fac+nf)->nofac_fichier, cir_emetteur, normale_emetteur); 
   					
					//printf("somme fac forme %f \n",somme_fac_form);
      				if( somme_fac_form > max_somme_fac )
					   { max_somme_fac = somme_fac_form;
  	   					 nof_max_somme=(fac+nf)->nofac_fichier;
 						 noc_max_somme=noc;
					   }

					fprintf (pval,"%10.3f\n",somme_fac_form);
					/* calcule 1-somme_fac_form */  
					somme_fac_form=1-somme_fac_form; if(somme_fac_form < 0) somme_fac_form=0;
					fprintf (pval1,"%10.3f\n",somme_fac_form);  
			     }

            pcont=pcont->suc; /* Passage au contour suivant de la face courante */
          }
  			
   }
 printf("somme fac forme MAXI calcule : %f pour face %d contour %d\n",max_somme_fac,nof_max_somme,noc_max_somme);

}
/*_________________________________________________________________*/
void fait_pers_fac_visib(nbfac_masque, fac_masque, fac_visib, fac0)
int nbfac_masque;
struct modelisation_face *fac_masque;
struct modelisation_face *fac_visib;
struct modelisation_face *fac0;
{  
 int i;

/* COPIE ORIGINAL fac_masque ds fac_visib pour travail */
       	for(i=0;i<nbfac_masque;i++) 
		copie_face(fac_masque+i,fac_visib+i);

/* TRANSFORMATION fichier "masque" et COUPE PYRAMIDE , si vu */
       	for(i=0;i<nbfac_masque;i++)
	  	{ 
		  //if(coplanaire(fac0,1,fac_masque+i,1,epsilone)!=1)
		  if(!(coplanaire(fac0,1,fac_masque+i,1,epsiloneN, 0,facteurD)))

          //if((fac_visib+i)->nofac_fichier !=noface)
              		{ //printf("Face %d Non coplanaire\n", (fac_visib+i)->nofac_fichier );

			          if(visible_pers(fac_visib+i,1))
                   		{ 
	                     	tran_face(fac_visib+i,1,fac_visib+i,0);
                     		tran_normale((fac_visib+i)->vnorm);
                     		if((fac_visib+i)->debut_dessin) 
                        		{ 
					             calcul_d_du_plan(fac_visib+i,0);
                          		 face_dans_vision(fac_visib+i,0); 
                        		}
                   		}
               		}
          	}

/* PERSPECTIVE */
       init_fenetre_affichage();
       for(i=0;i<nbfac_masque;i++)
		{if((fac_visib+i)->debut_dessin)
               		{pers_conic_face(fac_visib+i,0);}
		}
               	
/* reajuste la fenetre a angle de vision */
       fen_aff[0]=-tgvis; fen_aff[1]=-tgvis; 
       fen_aff[3]=tgvis; fen_aff[4]=tgvis; 
       cal_fen_aff();
              /* attention si angvis proche de 90 */
              /* on evite fen_aff[6]=0 */
       if(fen_aff[6]<0.008) fen_aff[6]=0.008;


/* NORMALISATION */
       z_axono_pers=1;  
       option_calcul_z=0;

       for(i=0;i<nbfac_masque;i++)
	    { if((fac_visib+i)->debut_dessin)
               { normalise_face(fac_visib+i,0);
                 sens_face(fac_visib+i,0,1);		 
               }

	      copie_face_projete(fac_visib+i,0,fac_visib+i,1);
        }

  /* TRAITE VU/CACHE ; resultat dans ->dessin */

       traite_moins(fac_visib,nbfac_masque);
       z_axono_pers=0;
       option_calcul_z=1;

/*
       printf("Les faces visibles apres moins\n");
	   for(i=0;i<nbfac_masque;i++)
        {
		  if((fac_visib+i)->debut_dessin)
	       { 
			printf("Face %d visible\n", (fac_visib+i)->nofac_fichier );
		   }
	    }
*/

}


/*_________________________________________________________________*/
void preevalue_ff_visib(nbfac, fac, nbfac_masque, fac_visib, fac_masque, noface, cir_emetteur, normale_emetteur) 
/* Numero de la face contenant le contour a traiter */
int nbfac;
struct modelisation_face *fac;
int nbfac_masque;
struct modelisation_face *fac_visib;
struct modelisation_face *fac_masque;
int noface;
struct circuit *cir_emetteur;
struct vector normale_emetteur;
{
  /* pour chaque face de fac (nbfac) */
  /* cherche si vue ds la perspective fac_visib (nbfac_masque) */
  /* si vu, test pour chaque centre de gravite des contours de fac */
  /* si est dans le polygone image perspective fac_visib de la face */
 
 int i,j;
 float facf_nul=0.0;
 float *fac_de_form;

 float *ff_nul;
 

for(i=0;i<nbfac;i++)
  	{ //printf(" i=%d examine face %d\n",i,(fac+i)->nofac_fichier);
	 if((fac+i)->nofac_fichier == noface) // Ok car no unique des faces ds meme fichier
       { 
		/* cette face numero i contient le contour en traitement */
		/* -->>> generer autant de facteurs de forme nuls que de contours */
		  //printf("identique à face en traitement\n");

// MODIF 15 JUILLET 2005 D.GROLEAU
		  //écrit en une seule fois sur disque des ff nuls
		  ff_nul = alloue_float(nb_contour_face(fac+i,1),345);
		  fwrite(ff_nul,sizeof(float),nb_contour_face(fac+i,1),pform);
		  desalloue_float(ff_nul);

/* anciennement
	  	for(k=0;k<nb_contour_face(fac+i,1);k++)
		  {// printf("... avec face %d contour %i, non visible niv.1\n",(fac+i)->nofac_fichier, k+1); 
			if(sortie) fprintf(pform," f%d c%d  %d\n",(fac+i)->nofac_fichier,k+1,0);
			else fwrite(&facf_nul,sizeof(float),1,pform);		
	      }
*/
// FIN MODIF 15 JUILLET 2005
		}
      else
      	{  
		    /* c'est une face differente de la face */
         	/* qui contient le contour en traitement */ 
         	/* cherche si vue ds pers */

			// alloue et initialise  fac_de_forme pour le contour en traitement
			fac_de_form = alloue_float(nb_contour_face(fac+i,1),1234);

        	for(j=0;j<nbfac_masque;j++)
         	  { 
			    if((fac_visib+j)->debut_dessin)
            	 { //printf("    test coplanaire avec %d\n",(fac_visib+j)->nofac_fichier);
				   //if(coplanaire(fac+i,1,fac_masque+j,1,epsilone))
				   if(coplanaire(fac+i,1,fac_masque+j,1,epsiloneN, 1,facteurD))
					{ 
					 /* elle est vue au moins en partie */
					 /* mais ATTENTION, la face peut être coplanaire avec d'autres aussi */
					 /* il faut donc poursuivre jusqu'au bout */

					 //printf("coplanaire avec face visible %d\n",(fac_visib+j)->nofac_fichier);
					 //printf("  lancement calcul pour face %d\n", (fac+i)->nofac_fichier ); 

					 ff_des_patch_ds_face(fac, fac_visib,i,j, cir_emetteur, normale_emetteur,fac_de_form); 
					}
            	 }
              }

// MODIF 15 JUILLET 2005 D.GROLEAU
		  	  // generer autant de fac_form que de contours 
			  // ecrit en une seule fois
			  fwrite(fac_de_form,sizeof(float),nb_contour_face(fac+i,1),pform);
/* anciennement
	  	  	  for(k=0;k<nb_contour_face(fac+i,1);k++)
			  {
				 if(sortie) fprintf(pform," f%d c%d  %8.6f\n",(fac+i)->nofac_fichier,k+1,fac_de_form[k]); 
				 else fwrite(&(fac_de_form[k]),sizeof(float),1,pform);
			  }
*/

// FIN MODIF 15 JUILLET 2005

            desalloue_float(fac_de_form);

       }// fin de else
  	}// fin for i

}

/*_________________________________________________________________*/
void ff_des_patch_ds_face(fac, fac_visib, i,j, cir_emetteur, normale_emetteur,fac_de_form)
struct modelisation_face *fac;
struct modelisation_face *fac_visib;
int i,j;
struct circuit *cir_emetteur;
struct vector normale_emetteur;
float *fac_de_form;
{ /* Calcul des facteurs de forme des differents contours de la face i */
  /* par rapport au contour de reference 'cir_emetteur' */
 
 int noc;
 struct contour *pcont;
 struct circuit *pcir;
 double xg,yg,zg;
 double xp, yp, zp;
 double xyz[3];
 float ff = 0.0;
 double ang_inc;
 int test_nul = 0;


   noc=0;
   pcont=(fac+i)->debut_projete;
   while(pcont)
     { pcir=pcont->debut_support;
	   noc++;
       //printf("... avec face %d contour %i\n",(fac+i)->nofac_fichier, noc); 

       /* cherche centre de gravite du contour courant */
	   centre_de_gravite(pcir,&xg,&yg,&zg);

      /* et vecteur centre de gravite - point observation */
           
        xg=xg-obs.xo;
        yg=yg-obs.yo;
        zg=zg-obs.zo;

      /* test si ds le champ de la face */
      xyz[0]=xg; xyz[1]=yg; xyz[2]=zg;

      if((vincid(vnf,xyz,&ang_inc)) > 0)
       {

        /* transforme en perspective */
      	tranp(xg,yg,zg,xyz,xyz+1,xyz+2);

        /* coupe par pyramide : retient ou non le point */
       	if(xyz[2]<0 && fabs(xyz[0]/xyz[2])<tgvis && fabs(xyz[1]/xyz[2])<tgvis) 
          { 
            /* met en pers et normalise  */
            xp=-xyz[0]/xyz[2];
            yp=-xyz[1]/xyz[2];
            zp=0;
                /*printf("xa %f  ya %f  za %f\n",xp,yp,zp);*/

            z_axono_pers=1;
            normalise_point(xp,yp,zp,&xp,&yp,&zp);
            z_axono_pers=0;
                /*printf("xp %f  yp %f  zp %f\n",xp,yp,zp);*/

            test_nul=0;

            /* test si ds face j en pers */
            if(point_dans_face(xp,yp,(fac_visib+j),0))
             {
		      ff=(float)calcul_ff(cir_emetteur, normale_emetteur, pcir); /* Envoi du contour courant a la fonction de calcul du f.f. */
              fac_de_form[noc-1] = ff;
		      somme_fac_form+=ff;
             }
           }
     }

   pcont=pcont->suc;
 }

}

/*_________________________________________________________________*/

void ne_voit_rien(nbfac,fac)
int nbfac;
struct modelisation_face *fac;
{
 int i,k;
 float ff_nul = 0.0;

 // generer autant de fac_form=0 que de contours 
 for(i=0;i<nbfac;i++)
  	{ 
      for(k=0;k<nb_contour_face(fac+i,1);k++)
		{
				 if(sortie) fprintf(pform," f%d c%d  %8.6f\n",(fac+i)->nofac_fichier,k+1,ff_nul); 
				 else fwrite(&ff_nul,sizeof(float),1,pform);
		}
    }

 fprintf (pval," 0.0\n",somme_fac_form);
 /* la somme des fcateurs de forme hors scene est aussi 0 */  
 fprintf (pval1," 0.0\n",somme_fac_form);

}
/*_________________________________________________________________*/

/****** Modifs MJA 1998 *****/

/*_________________________________________________________________*/
struct circuit* alloue_cir(nb_points)
int nb_points;
{ struct circuit *pcir_nouv;
  int OK_alloc_points;
pcir_nouv=(struct circuit*)malloc(sizeof(struct circuit));
if (pcir_nouv==NULL)
	{ printf("--- Probleme allocation nouveau circuit ...\n");
	  return NULL;}

/* Allocation points du circuit */

pcir_nouv->nbp=nb_points; 
OK_alloc_points = alloue_point_circuit(pcir_nouv, 1);
if (!OK_alloc_points) {printf("--- Probleme allocation points nouveau circuit...\n");
				 return NULL;}
			 
pcir_nouv->suc = NULL;
return pcir_nouv;
}

/*_________________________________________________________________*/
void desalloue_cir(pcir)
struct circuit *pcir;
{ if (pcir->nbp)
	{free(pcir->x);
	 free(pcir->y);
	 free(pcir->z);
	}
 pcir->nbp=0;
 pcir->x = NULL;
 pcir->y=NULL;
 pcir->z=NULL;

 free(pcir);
 pcir=NULL;
}


/*_________________________________________________________________*/
double distance_entre_patchs(pcir1, pcir2)
struct circuit *pcir1, *pcir2;
{ double dpp;
double xg1, yg1, zg1;
double xg2, yg2, zg2;

xg1=0.0;yg1=0.0;zg1=0.0;
xg2=0.0;yg2=0.0;zg2=0.0;

centre_de_gravite(pcir1, &xg1, &yg1, &zg1); /* REF */
centre_de_gravite(pcir2, &xg2, &yg2, &zg2); /* PCIR */


dpp = 0.0;
dpp = (xg1-xg2)*(xg1-xg2) + (yg1-yg2)*(yg1-yg2) + (zg1-zg2)*(zg1-zg2);
dpp = sqrt(dpp);

return dpp;

}


/*_________________________________________________________________*/
double longueur_arete_max(pcir)
struct circuit *pcir;
{double lga, lga_mx;
 int k;

lga_mx=0.0;
		 	
for(k=0;k<pcir->nbp-1;k++) 
        { 
	lga=longueur_arete_poly(pcir->x[k],pcir->y[k],pcir->z[k],pcir->x[k+1],pcir->y[k+1],pcir->z[k+1]);
	if(lga>lga_mx)lga_mx=lga;
        }
return lga_mx;
}

/*_________________________________________________________________*/
void affiche_tri(tri)
struct circuit *tri;
{int i;
for (i=0;i<=2;i++)
	printf("Point %d: X=%lf, Y=%lf, Z=%lf\n", i, tri->x[i], tri->y[i], tri->z[i]);
}
/*_________________________________________________________________*/
double norme_vect3D(x, y, z)
double x, y, z;
{return sqrt(x*x+y*y+z*z);}
/*_________________________________________________________________*/
void prod_vect_3D(x1, y1, z1, x2, y2, z2, x, y, z)
double x1, y1, z1;
double x2, y2, z2;
double *x, *y, *z;
{/* Calcule les composantes du produit vectoriel de 2 vecteurs '1' et '2' */
*x = y1*z2-y2*z1;
*y = z1*x2-z2*x1;
*z = x1*y2-y1*x2;
}
/*_________________________________________________________________*/
double prod_scal_3D(x1, y1, z1, x2, y2, z2)
double x1, y1, z1;
double x2, y2, z2;

{/* Calcule le produit scalaire de 2 vecteurs '1' et '2' */
return x1*x2 + y1*y2 + z1*z2;
}

/*_________________________________________________________________*/
int sinus_vect3D(x1, y1, z1, x2, y2, z2, sinus) /* Renvoie 1 si calcul OK */
double x1, y1, z1;
double x2, y2, z2;
double *sinus;
{
 double norm1, norm2;
 double x,y,z;
 double eps = 1e-8;
 int calculOK=1;

norm1 = norme_vect3D(x1, y1, z1);
norm2 = norme_vect3D(x2, y2, z2);

if (norm1< eps) 
	{printf("Erreur, 1er vecteur de norme nulle dans calcul sinus 3D ...\n");
	calculOK=0;
	}
if (norm2< eps) 
	{printf("Erreur, 2nd vecteur de norme nulle dans calcul sinus 3D ...\n");
	 calculOK=0;
	}
if (calculOK)
	{x = 0.0; y=0.0; z=0.0;
	 prod_vect_3D(x1,y1,z1,x2,y2,z2, &x,&y,&z);

	 *sinus = norme_vect3D(x,y,z)/(norm1*norm2);
	 }
else
	{*sinus=0;}

return calculOK;
}



/*_________________________________________________________________*/
double surface_tri(tri)
struct circuit *tri;
{ 
  double sin_angle=0.0;
  int OK=1;
  double cote1, cote2;
  double v1x, v1y, v1z, v2x, v2y, v2z;

if (tri->nbp != 4)
	{printf("Erreur, circuit non triangulaire ...\n");
	 exit(0);
	}

v1x = tri->x[1]-tri->x[0];
v1y = tri->y[1]-tri->y[0];
v1z = tri->z[1]-tri->z[0];
v2x = tri->x[2]-tri->x[0];
v2y = tri->y[2]-tri->y[0];
v2z = tri->z[2]-tri->z[0];

cote1 = norme_vect3D(v1x,v1y,v1z);
cote2 = norme_vect3D(v2x,v2y,v2z);

OK = sinus_vect3D(v1x, v1y, v1z, v2x, v2y, v2z, &sin_angle);

if (OK) return (0.5*cote1*cote2*sin_angle);
else return 0.0;
}

/*_________________________________________________________________*/
int cosinus_vect3D(x1, y1, z1, x2, y2, z2, cosinus) /* Renvoie 1 si calcul OK */

double x1, y1, z1;
double x2, y2, z2;
double *cosinus;
{ 
 double norm1, norm2;
 double eps = 1e-8;
 int calculOK=1;

norm1 = norme_vect3D(x1, y1, z1);
norm2 = norme_vect3D(x2, y2, z2);

if (norm1<eps) 
	{printf("Erreur, 1er vecteur de norme nulle dans calcul cosinus 3D ...\n");
	 calculOK=0; 
	}
if (norm2<eps) 
	{printf("Erreur, 2nd vecteur de norme nulle dans calcul cosinus 3D ...\n");
	 calculOK=0;
	}

if (calculOK) *cosinus = prod_scal_3D(x1,y1,z1,x2,y2,z2)/(norm1*norm2);
else *cosinus=0;

return calculOK;
}

/*_________________________________________________________________*/
void arrange_points(cir_ref, tri1, tri2)
struct circuit *cir_ref, *tri1, *tri2;
/* Remplit coordonnees points de 2 triangles */
/* crees a partir du contour de depart (ici, un quadrangle) */
/* peut etre generalise a n sommets */
{ if (cir_ref->nbp == 5)
	{tri1->x[0] = cir_ref->x[0];
	 tri1->x[1] = cir_ref->x[1];
	 tri1->x[2] = cir_ref->x[2];
	 tri1->x[3] = cir_ref->x[0];
	 tri1->y[0] = cir_ref->y[0];
	 tri1->y[1] = cir_ref->y[1];
	 tri1->y[2] = cir_ref->y[2];
	 tri1->y[3] = cir_ref->y[0];
	 tri1->z[0] = cir_ref->z[0];
	 tri1->z[1] = cir_ref->z[1];
	 tri1->z[2] = cir_ref->z[2];
	 tri1->z[3] = cir_ref->z[0];

	 tri2->x[0] = cir_ref->x[0];
	 tri2->x[1] = cir_ref->x[2];
	 tri2->x[2] = cir_ref->x[3];
	 tri2->x[3] = cir_ref->x[0];
	 tri2->y[0] = cir_ref->y[0];
	 tri2->y[1] = cir_ref->y[2];
	 tri2->y[2] = cir_ref->y[3];
	 tri2->y[3] = cir_ref->y[0];
	 tri2->z[0] = cir_ref->z[0];
	 tri2->z[1] = cir_ref->z[2];
	 tri2->z[2] = cir_ref->z[3];
	 tri2->z[3] = cir_ref->z[0];
	}
 else
	{printf("Attention, circuit non quadrangulaire ...\n");}
}
	 

/*_________________________________________________________________*/
double calcul_ff(cir_emetteur, normale_emetteur, pcir)
struct circuit *cir_emetteur;
struct vector normale_emetteur;
struct circuit *pcir;
/* Calcul du facteur de forme entre deux contours: 	*/
/* le contour de reference 'cir_emetteur' 		*/
/* et le contour courant 'pcir' 			*/
{ double facf; 
  
  if (cir_emetteur->nbp == 4)
	{/*printf("Interface: triangle\n");*/ 
	 facf = traite_triangle(cir_emetteur, normale_emetteur, pcir);}

  else if (cir_emetteur->nbp == 5)
	{/*printf("Interface: quadrangle\n"); */
         facf = traite_quadrangle(cir_emetteur, normale_emetteur, pcir);
	 }
  else
	{/*printf("Interface: nb de points superieur a 4\n"); */
	 facf = 0.0;}

  return facf;
}
/*_________________________________________________________________*/
double traite_triangle(cir_ref, normale_emetteur, pcir)
struct circuit *cir_ref;
struct vector normale_emetteur;
struct circuit *pcir;
{ /* C'est l'ancienne fonction 'traite_nusselt' */
  /* Le circuit de reference est maintenant passe en parametre */

  int i,recurs, nbdivise=1;
  struct triangle t;
  double valg;
  double xg, yg, zg;
  double distance_pp, arete_max;

  /* Methode de YAMANOUTI */
 

/* triangle parent */
/* ne fonctionne que pour patch triangulaire */

  if (cir_ref->nbp != 4)
	{printf("Erreur, circuit non triangulaire ...\n");
	 return 0.0;}

  t.a.x=cir_ref->x[0],t.a.y=cir_ref->y[0], t.a.z=cir_ref->z[0];
  t.b.x=cir_ref->x[1],t.b.y=cir_ref->y[1], t.b.z=cir_ref->z[1];
  t.c.x=cir_ref->x[2],t.c.y=cir_ref->y[2], t.c.z=cir_ref->z[2];

  centre_de_gravite(cir_ref, &xg, &yg, &zg);
  arete_max = longueur_arete_max(cir_ref);
  distance_pp = distance_entre_patchs(cir_ref, pcir);
 
  if(distance_pp > 5*arete_max) /* Critere d'application de Yamanouti */
	{ 
	valg=fform_yamanouti(xg,yg,zg, normale_emetteur,pcir);
	recurs=0;
	}
  else if(distance_pp > 5*arete_max/2) recurs=1;
  else if(distance_pp > 5*arete_max/4) recurs=2;
  else recurs=3;

  /* printf("Traite_triangle: niveau de recursivite %d\n", recurs); */

  if(recurs)
   	{ 
	valg=0.0;

	/* divise le triangle parent en triangles plus petits */
	/* et calcule le facteur de forme en sommant les contributions correspondantes */

	divise_triangle(&t, recurs, normale_emetteur, pcir, &valg); 

	/* Moyenne sur les 'nbdivise' triangles crees */
	for (i=0;i<recurs;i++)
		nbdivise *= 4;
	valg=valg/nbdivise;
   	}    

return(valg);
}

/*________________________________________________________________________*/
double traite_quadrangle(cir_ref, normale_emetteur, pcir)
struct circuit *cir_ref;
struct vector normale_emetteur;
struct circuit *pcir;
{ double valg, facft1, facft2;
  double aire_t1, aire_t2;
  struct circuit *tri_aux1, *tri_aux2;
  double eps = 1e-8;
  
/* On forme deux triangles a partir du quadrangle de depart */
/* et on ajoute les facteurs de forme */

  tri_aux1 = alloue_cir(4);
  tri_aux2 = alloue_cir(4);
 
  arrange_points(cir_ref, tri_aux1, tri_aux2);
  aire_t1 = surface_tri(tri_aux1);   
  aire_t2 = surface_tri(tri_aux2);   
  facft1 = traite_triangle(tri_aux1, normale_emetteur, pcir);   
  facft2 = traite_triangle(tri_aux2, normale_emetteur, pcir); 
/* Moyenne ponderee par la surface des 2 triangles */
  if ((aire_t1+aire_t2)<eps) 	valg = 0.0;
  else 				valg = (aire_t1*facft1 + aire_t2*facft2)/(aire_t1+aire_t2);
  
/*  printf("ff calcule: %lf\n", valg); */

  desalloue_cir(tri_aux1);
  desalloue_cir(tri_aux2);

return valg;
}

/*________________________________________________________________________*/
/* divise un triangle en 4 triangles plus petits */
/* et calcule les facteurs de forme 'elementaires' correspondants */
/* 'valg' n'st plus une variable globale, mais est passee en parametre */

void divise_triangle(t, profond, normale_emetteur, pcir, valg)
struct triangle *t;
int profond;
struct vector normale_emetteur;
struct circuit *pcir;
double *valg;

	{
	int subt; 
	double cdgx,cdgy,cdgz,val;
	struct triangle new_t[4];
	double x[3], y[3], z[3];	
	
	val=0;
	
	/* Creation des 4 nouveaux triangles */
	new_t[0].a = t->a;
	new_t[1].b = t->b;
	new_t[2].c = t->c;
	new_t[0].c = new_t[2].a = new_t[3].b = average(&t->a, &t->c);
	new_t[0].b = new_t[1].a = new_t[3].c = average(&t->a, &t->b);
	new_t[1].c = new_t[2].b = new_t[3].a = average(&t->c, &t->b);
	
	/* recommence si profond n'est pas atteint */
	if (--profond != 0)
		{for(subt = 0; subt < 4; subt++)
			divise_triangle(&new_t[subt], profond, normale_emetteur, pcir, valg);}
	else            
	    	{
	    	for(subt = 0; subt < 4; subt++)
			{
	        	x[0]=new_t[subt].b.x, x[1]=new_t[subt].a.x, x[2]=new_t[subt].c.x;
	        	y[0]=new_t[subt].b.y, y[1]=new_t[subt].a.y, y[2]=new_t[subt].c.y;
	        	z[0]=new_t[subt].b.z, z[1]=new_t[subt].a.z, z[2]=new_t[subt].c.z;
			/* centre de gravite */
			cdgx=(x[0]+x[1]+x[2])/3,cdgy=(y[0]+y[1]+y[2])/3,cdgz=(z[0]+z[1]+z[2])/3;
			/* facteur de forme */
			val+=fform_yamanouti(cdgx,cdgy,cdgz,normale_emetteur,pcir);
			}
	    	(*valg)+=val;
	    	}
	}


/*-----------------------------------------------------------*/
double fform_yamanouti(xppp, yppp, zppp,normale_emetteur,pcir)

double xppp,yppp,zppp; /* Les coordonnees du centre de gravite de l'emetteur */
struct vector normale_emetteur;
struct circuit *pcir;	/* Le contour recepteur */
{
  int i;
  double aire,ga,gb;
  double poab[4],xt[3],yt[3],zt[3];
  double normal_x, normal_y, normal_z;
  int OKalpha=1;
  int OKbeta=1;
  int OKcalcul=1;
  double facf=0.0;

/* Implementation de l'expression du calcul du facteur de forme */
/* Approximation point-polygone */

  aire=0;

  xt[2]=xppp;  yt[2]=yppp; zt[2]=zppp;
  normal_x = normale_emetteur.cx;
  normal_y = normale_emetteur.cy;
  normal_z = normale_emetteur.cz;

  for(i=0;i<pcir->nbp-1;i++)
    { 	/* traite une arete ab (i,i+1) */

	OKalpha=1; OKbeta=1;

       	xt[0]=pcir->x[i]; yt[0]=pcir->y[i]; zt[0]=pcir->z[i];  
       	xt[1]=pcir->x[i+1]; yt[1]=pcir->y[i+1]; zt[1]=pcir->z[i+1]; 
 
      	/* normale du plan oab */       
       	normale_avec_3pts(xt,yt,zt,poab);
	
      	/* calcul alpha  : cos entre 2 vecteurs poab et normale normale_x y z */\
	
 	ga=0.0;
       	OKalpha=cosinus_vect3D(poab[0],poab[1],poab[2],normal_x,normal_y,normal_z,&ga);

	gb=0.0;
	OKbeta=cosinus_vect3D(xt[0]-xt[2],yt[0]-yt[2],zt[0]-zt[2],xt[1]-xt[2],yt[1]-yt[2],zt[1]-zt[2],&gb);
	
	if (gb > 1.0) gb = 1.0;
	if (gb <-1.0) gb = -1.0; 
	
	if (OKalpha*OKbeta)
     		aire+=ga*acos(gb); 

	OKcalcul = OKcalcul*OKalpha*OKbeta;
		 
    }

  if (OKcalcul) 
	facf=fabs(aire/2)/pi;
  else 
	facf=0.0;

   return facf;
}
/****** Fin des modifs MJA 1998 *****/
/*________________________________________________________________________*/
/* milieu de 2 points */
struct vertex average(v1, v2)
struct vertex *v1;
struct vertex *v2;
	{
	struct vertex m;

	m.x = (v1->x + v2->x) / 2;
	m.y = (v1->y + v2->y) / 2;
	m.z = (v1->z + v2->z) / 2;

	return m;
	}

/*________________________________________________________________________*/
double longueur_arete_poly(x1,y1,z1,x2,y2,z2)
double x1,y1,z1,x2,y2,z2;
{
 double longueur;
 longueur=(sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1)));
/*printf("longueur arete %f\n",longueur);*/
 return(longueur);
 
}


/*_________________________________________________________________*/
void format_facform()
{
 printf("\n   format d'entree des parametres \n\n");

 printf("\n  *facform*  geometrie_in(.cir)  geometrie_masque_in(.cir) binaire_ou_ascii facteurs_de_forme-out(.fac)  somme_fac_form_scene_out(.val) somme_fac_form_ciel_out(.val) [epsilone de planimetrie] [1/2angle de vision]\n\n");

 printf("\n      la fonction a comme parametre ENTREE :\n\n");
 printf("\t geometrie_a_simuler_in(.cir)\n"); 
 printf("\t geometrie_masque_in(.cir)\n");
 printf("\t facteurs de forme en binaire(0) ou ascii(1)\n");

 printf("\n                comme parametres en SORTIE :\n\n");

 printf("\t  fichier des n*n facteurs de forme(.fac)\n\n");
 printf("\t  descripteur: somme_fac_form_scene(.val)\n\n");
 printf("\t  descripteur: somme_fac_form_hors_de_la_scene(.val)\n");
       
  printf("\n      la fonction a comme OPTION ENTREE :\n\n");
  printf("\t epsilone de planérité des faces (évalué sur d; défaut : 0.001)  \n\n");
  printf("\t 1/2 angle de vision (défaut : 89.9)  \n\n");

  	exit(0);
}
