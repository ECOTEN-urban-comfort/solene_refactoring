/* facform_ciel.c : calcul facteur de forme avec le ciel	*/
/*______________________________________________________________________*/


//D. GROLEAU modification mars 2004 // appel à coplanaire()
							        // epsiloneN =0.0001 defautD=1000
// modele : fraction_ciel et df_visib_facform
/*
cc facform_ciel.c pers_util.o solutile.o geomutile.o face_op_face.o poly_op_poly.o traite.o lib_solene_94.o solaire.o -o facform_ciel -lm
*/

//si recompile, vérifier nb de paramètres de coplanaire (sens de la normale)


/* POUR UN FICHIER .cir, determine, pour une face et un contour donnes,	le facteur de forme
avec les portions de ciel 
vues depuis le centre de gravite de ce contour, en tenant compte des masques eventuels 
*/

// ne tient pas compte des faces situés dans le même plan que celui du contour en traitement

#include<solene.h>

// declaration fonctions
double fform_yamanouti();
void patch_gravite();     
double masque_ciel();
void ne_voit_rien();
int test_si_patch_vu();
double traite_nusselt();
void  traite_facform_ciel();
void usage_facform_ciel();

//GLOBAL
struct modelisation_face *fac0 ;	/* geometrie a evaluer */
struct modelisation_face *fac1;		/* geometrie des masques */
struct modelisation_face *fac2;		/* geometrie du ciel */

FILE *pval,*pval1,*pval2;


extern int option_calcul_z;   	/* option CALCUL du Z ds FACE-OP-FACE 1=oui */
                              	/* utilise ds singul.c epure_polygone */
			      	/* et dans face_op_face.c             */
extern double coef_discol;


int im,nbre_patch_vu;
double vnf[3];   		/* normale a la face en traitement */
double tgvis,covis,sinvis;
double mini,maxi,*xg_patch,*yg_patch,*zg_patch;
double *vnf2_x,*vnf2_y,*vnf2_z;
double normal_x,normal_y,normal_z;

double epsiloneN;   	// pour appel coplanaire
int    defautD;			// pour appel coplanaire

/*_________________________________________________________________*/
main(argc,argv)           /* MASQUE  pour un FICHIER DE FACES */
int argc;char **argv;
	{
	int i,nbfac0,nbfac1,nbfac2,nb_contour;
	int nomax0;

 	double ang;

 	char nom_in[256],nom_masc[256],nom_ciel[256],nom_out[256];
	char *s_dir;

 	FILE *pfic;
	double englob[10];

	/* initialisation */
   	singularite=0; non_singularite=0; nb_etat=0;
   	pb_masque=0; colle_max=coef_discol*DISCOL;
   	pi=4*atan(1.);

	// pour appel coplanaire
	epsiloneN=0.0001;
	defautD=1000;
	
	s_dir=(char *)getenv("PWD");

   	if(argc!=6) usage_facform_ciel();
 
/* lecture parametres commande */
	compose_nom_complet(nom_in,s_dir,argv[1],"cir");
    printf("\n  fichier a traiter : %s \n", nom_in);

    compose_nom_complet(nom_masc,s_dir,argv[2],"cir");  
    printf("  fichier masque : %s \n", nom_masc);

	compose_nom_complet(nom_ciel,s_dir,argv[3],"cir");
    printf("  discretisation du ciel : %s \n", nom_ciel);
	
    sscanf(argv[4],"%lf",&ang);
	if(ang<0 || ang>89.99 || ang<15) 
	  { 	
		printf("   *** 15 < angle_vision < 89.99 ***\n");
	    exit(0);
	  }
    ang=pi*ang/180.;
    tgvis=tan(ang); covis=cos(ang); sinvis=sin(ang);


/****LECTURE DES FICHIERS*** */
/** Lecture du fichier CIEL - indice 2 **/
	if ((pfic=fopen(nom_ciel,"r"))==NULL)
      { 
		printf("\n  impossible ouvrir %s\n\n", nom_ciel); 
		exit(0);
      }
    lit_en_tete(pfic,&nbfac2, &i, englob);
    fac2=alloue_face(nbfac2, 1000);
    lit_fic_cir3d(pfic, nbfac2, fac2);

    fclose(pfic);

	/* calcul centre gravite patch de ciel */
  	xg_patch=alloue_double(nbfac2,123);
  	yg_patch=alloue_double(nbfac2,124);
  	zg_patch=alloue_double(nbfac2,125);
  	patch_gravite(nbfac2);

 /*** Lecture du fichier GEOM a traiter  ***/

    if ((pfic=fopen(nom_in,"r"))==NULL)
      { 
	    printf("\n impossible ouvrir %s\n\n", nom_in); 
	    exit(0);
      }

    lit_en_tete(pfic,&nbfac0, &nomax0, englob);

	/* alloue les faces  a traiter*/
   fac0=alloue_face(nbfac0,34);
   lit_fic_cir3d(pfic,nbfac0,fac0);
   fclose(pfic);

   nb_contour= nbcontours_total(fac0, nbfac0);

   printf("\n Traite %d faces,  soit %d contours\n",nbfac0,nb_contour);

/*** Lecture du fichier GEOM MASQUE - indice 1 */
    if ((pfic = fopen(nom_masc,"r"))==NULL)
      { 
	    printf("\n impossible ouvrir %s\n", nom_masc); 
	    exit(0);
      }
    lit_en_tete(pfic,&nbfac1, &i, englob);
    printf(" avec masque de %d faces\n\n", nbfac1);
    fac1=alloue_face(nbfac1, 34);
    lit_fic_cir3d(pfic, nbfac1, fac1);
    fclose(pfic);

	/* stocke les normales de fac1 */
	vnf2_x=alloue_double(nbfac1,127);
	vnf2_y=alloue_double(nbfac1,128);
	vnf2_z=alloue_double(nbfac1,129);

    for(i=0;i<nbfac1;i++)
      { 
		vnf2_x[i]=(fac1+i)->vnorm[0];
		vnf2_y[i]=(fac1+i)->vnorm[1];
		vnf2_z[i]=(fac1+i)->vnorm[2]; 
	  }


/*** Le fichiers RESULTAT ***/
        
	mini=0; maxi=100;
	compose_nom_complet(nom_out,s_dir,argv[5],"val");  
    printf("  fichier resultat facform avec ciel : %s \n\n", nom_out);
	if ((pval = fopen(nom_out,"w")) ==NULL)
      { 	
		printf("\n impossible ouvrir %s\n", nom_out); 
		exit(0);
       }
	fprintf (pval,"%5d %5d %10.2f %10.2f\n", nbfac0, nomax0, mini, maxi);   

/* observateur regarde vers le haut a verticale de oeil */
    obs.x=0; obs.y=0; obs.z=-1; 
    

/* TRAITEMENT POUR CHAQUE CONTOUR DE CHAQUE FACE de GEOM */

    printf("Traitement en cours ...\n");
    traite_facform_ciel(nbfac0,nbfac1,nbfac2);

/* FIN du TRAITEMENT                                     */

    desalloue_fface(fac1,nbfac1);
	desalloue_fface(fac0,nbfac0);
    desalloue_fface(fac2,nbfac2);

	desalloue_double(xg_patch);
	desalloue_double(yg_patch);
	desalloue_double(zg_patch);

    desalloue_double(vnf2_x);
    desalloue_double(vnf2_y);
    desalloue_double(vnf2_z);

    fclose(pval); 

	creer_OK_Solene();
	printf("\n\nFin de facform_ciel\n");
	exit(0);
}

/*_________________________________________________________________*/
void traite_facform_ciel(nbfac0,nbfac1,nbfac2)
int nbfac0,nbfac1,nbfac2;
{
int i,nb_contour,noc;
struct contour *pcont;
struct circuit *pcir;
double valeur_calcul;


    for(i=0;i<nbfac0;i++)
      { 
	    //printf("Traitement de la face no %d\n",(fac0+i)->nofac_fichier);
        noc=0;

		vnf[0]=(fac0+i)->vnorm[0];
		vnf[1]=(fac0+i)->vnorm[1];
		vnf[2]=(fac0+i)->vnorm[2]; 

		normal_x=vnf[0];
		normal_y=vnf[1];
		normal_z=vnf[2];

		nb_contour=nb_contour_face(fac0+i,1);
		fprintf(pval,"f%d %d\n",(fac0+i)->nofac_fichier,nb_contour); 

         pcont=(fac0+i)->debut_projete; 
         while(pcont)	   
            { pcir=pcont->debut_support;

		      centre_de_gravite(pcir, &obs.xo, &obs.yo, &obs.zo);
              noc++;
			  //printf("Contour %d\n",noc);
              //printf("  %lf %lf %lf\n",obs.xo,obs.yo,obs.zo); 
			  
			  /* traitement préliminaire des coplanerités pour traiter le cas
				 frequent où 2 faces sont confondues en partie (2 pignons de bâtiments par ex)
              */
			  /* EN COMMENTAIRE
			  if(traite_coplanerite(obs.xo,obs.yo,obs.zo,fac0+i,fac1,nbfac1))
			     { 
					//l'observateur est masqué totalement
					printf("CACHE\n");
			        ne_voit_rien(nbfac2);
		         }
		      else */
		         {
                    valeur_calcul=masque_ciel(nbfac1,nbfac2,(fac0+i));
					//printf("  nombre de patches de ciel vus %d\n",nbre_patch_vu); 
			        //printf("  pourcent ciel vu %.1f\n",valeur_calcul*100);
		         }
			  pcont=pcont->suc;
			} 
        }
}

/*_________________________________________________________________*/
double masque_ciel(nbfac1,nbfac2,fac0)
int nbfac1,nbfac2;
struct modelisation_face *fac0;
{
	int i,vu;
	double cos_ang_inc, xyz[3], ang_inc;
	double valeur_calcul;
	struct contour *pcont;
	struct circuit *pcir;

  /* TRANSFORMATION fichier "masque" et COUPE PYRAMIDE , si vu */
   tranfo();
   for(i=0;i<nbfac1;i++)
	 { //if((fac1+i)->nofac_fichier !=noface)
	   	if(!(coplanaire(fac0,1,fac1+i,1,epsiloneN,0,defautD)))
          { if(visible_pers(fac1+i,1))
               { 
                 tran_face(fac1+i,1,fac1+i,0);
                 tran_normale((fac1+i)->vnorm);
                 if((fac1+i)->debut_dessin) 
                    { calcul_d_du_plan(fac1+i,0);
                      face_dans_vision(fac1+i,0);
                    }
               }
           }
      }

  /* PERSPECTIVE */
   init_fenetre_affichage();
   for(i=0;i<nbfac1;i++)
	  { if((fac1+i)->debut_dessin)
           { pers_conic_face(fac1+i,0);
           }
	  }

  /* reajuste la fenetre a angle de vision */
   fen_aff[0]=-tgvis; fen_aff[1]=-tgvis; 
   fen_aff[3]=tgvis; fen_aff[4]=tgvis; 
   cal_fen_aff();
          /* attention si angvis proche de 90 */
          /* on evite fen_aff[6]=0 */
   if(fen_aff[6]<0.008) fen_aff[6]=0.008;

   
   /* NORMALISATION */
   for(i=0;i<nbfac1;i++)
	  { if((fac1+i)->debut_dessin)
           { normalise_face(fac1+i,0);
           }
      }

/* ANALYSE DES PATCHES DE CIEL */
	nbre_patch_vu=0;
	valeur_calcul=0;

	for(i=0;i<nbfac2;i++)
      {  
		xyz[0]=xg_patch[i]; xyz[1]=yg_patch[i]; xyz[2]=zg_patch[i];
		cos_ang_inc=vincid(vnf,xyz,&ang_inc);
		if (cos_ang_inc>0)
		 {
       	   vu=test_si_patch_vu(nbfac1,xg_patch[i],yg_patch[i],zg_patch[i]);
		   nbre_patch_vu+=vu;
		   
           if(vu)
		   {
		     // calcul du facteur de forme avec ce patch de ciel
			  pcont=(fac2+i)->debut_projete;
			  pcir= pcont->debut_support;
			  valeur_calcul+= traite_nusselt(pcir);
		   }
		 }
      }

/* ECRITURE DU FICHIER .val */
	fprintf (pval," %8.6f\n", valeur_calcul);

/* reinverse la normale et desallocation face->dessin */  
    for(i=0;i<nbfac1;i++)
	  { (fac1+i)->vnorm[0] = vnf2_x[i];
        (fac1+i)->vnorm[1] = vnf2_y[i];
		(fac1+i)->vnorm[2] = vnf2_z[i];
        if((fac1+i)->debut_dessin)
              { desalloue_chaine_contour((fac1+i)->debut_dessin);
                (fac1+i)->debut_dessin=NULL;
              }
      }
return(valeur_calcul);
}
/*-----------------------------------------------------------------------------*/
int test_si_patch_vu(nbfac1,xg,yg,zg)
int nbfac1;
double xg,yg,zg;
{
	int in,ij;
 	double xyz[3];
	double xp, yp, zp;

    tranp(xg,yg,zg,xyz,xyz+1,xyz+2);
/* coupe par pyramide : retient ou non le point */

    if(xyz[2]<0 && fabs(xyz[0]/xyz[2])<tgvis && fabs(xyz[1]/xyz[2])<tgvis) 
        { 
                /* met en pers */
                xp=-xyz[0]/xyz[2];
                yp=-xyz[1]/xyz[2];
                zp=0;

                normalise_point(xp,yp,zp,&xp,&yp,&zp);

                 /* test si dans masque */
               	in=0;
               	for(ij=0;ij<nbfac1;ij++)
                 	{  
			          if((fac1+ij)->debut_dessin)
                       	{ 
				            in=point_dans_face(xp,yp,fac1+ij,0);
                         	if(in) return(0);
                       	}
                 	}
                /*printf("xp= %lf yp= %lf in= %d\n",xp,yp,in);*/
		        return(1);
	   	}
 	return(0);
}

/*-----------------------------------------------------------------------------*/
void ne_voit_rien(nbfac2)
int nbfac2;
{

 fprintf (pval,"%8.6f\n", 0.0);

}
		
/*-----------------------------------------------------------------------------*/
void patch_gravite(nbfac2)
int nbfac2;
	{
	int i;
 	struct contour *pcont;
 	struct circuit *pcir;

    for(i=0;i<nbfac2;i++)
        { 
            pcont=(fac2+i)->debut_projete;
	        pcir=pcont->debut_support;
		    centre_de_gravite(pcir, xg_patch+i,yg_patch+i,zg_patch+i);
     	}
	}

/* pris de facform */
/*_________________________________________________________________*/
double traite_nusselt(pcir)
struct circuit *pcir;
{ 
	
	// facteur de forme entre le contour en traitement et un patch de ciel visible
    // on considere le pb dans l'espace, le point dobservation en 0,0,0 et le ciel centré en 0
	double xgg,ygg,zgg;
	double(valg);

	xgg=0.0; ygg=0.0; zgg=0.0;
	valg=fform_yamanouti(pcir,xgg,ygg,zgg);

return(valg);
}


/*-----------------------------------------------------------*/
double fform_yamanouti(pcir,xppp,yppp,zppp)
struct circuit *pcir;
double xppp,yppp,zppp;
{
  int i;
  double aire,ga,gb;
  double poab[4],xt[3],yt[3],zt[3];

  aire=0;
/*  xt[2]=obs.xo;  yt[2]=obs.yo; zt[2]=obs.zo; */ /* pt O */

 xt[2]=xppp;  yt[2]=yppp; zt[2]=zppp;

  for(i=0;i<pcir->nbp-1;i++)
    { /* traite une arete ab (i,i+1) */
       xt[0]=pcir->x[i]; yt[0]=pcir->y[i]; zt[0]=pcir->z[i];  
       xt[1]=pcir->x[i+1]; yt[1]=pcir->y[i+1]; zt[1]=pcir->z[i+1]; 
 
      /* normale du plan oab */       
       normale_avec_3pts(xt,yt,zt,poab);
	
      /* calcul alpha  : cos entre 2 vecteurs poab et normale normale_x y z */
       ga=cos_angle_vec_3D(poab[0],poab[1],poab[2],normal_x,normal_y,normal_z);

      /* calcul du facteur de forme */

gb=cos_angle_vec_3D(xt[0]-xt[2],yt[0]-yt[2],zt[0]-zt[2],xt[1]-xt[2],yt[1]-yt[2],zt[1]-zt[2]);

       aire+=ga*acos(gb);  
    }

  return(fabs(aire/2)/pi);
}

/*_________________________________________________________________*/
/* Format de la fonction facform_ciel */
void usage_facform_ciel()
	{
  	printf(" *facform_ciel*   geom_in(.cir)  geom_masque(.cir)  ciel(.cir)  angle_vision   facform_avec_ciel(.val) \n\n");

 printf("\n la fonction a comme parametres en ENTREE :\n");
 printf("\t geometrie_in(.cir)\n"); 
 printf("\t geometrie_du_masque_in(.cir)\n"); 
 printf("\t ciel_in(.cir)\n"); 
 printf("\t angle_de_vision <90\n"); 
 printf("\t facteur_de_forme_avec_ciel_out(.val)\n\n"); 
	exit(0);
	}


