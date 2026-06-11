/* masques_ciel_lum.c : 

ENERGIE RECUE DU CIEL AVEC PRISE EN COMPTE DES MASQUES 
ET des TRANSPARENCES 

D. GROLEAU  mars 2000

D. GROLEAU  modifié le 18 novembre 2000 
	// pour tenir compte du nb de transmision à considérer
	// Ex: ne retenir seulement les énergies transmises après N transmissions
	// N=2 ne retient que l'énérgie transmise après 2 transmissions
	// si N est <0; considère toute l'énergie quelque soit le nb de transmissions

D. GROLEAU modifié décembre 2003 (gère les extension des fichiers en  Heure) 
									et application aux arbres volumes
									et simplification: on ne calcue que l'énergie reçue

D. GROLEAU  // mars 2004   appel a coplanaire

D. GROLEAU modification décembre 2005, traitement d'un plan fictif vertical (plan de coupe)
			avec calcul relatif à une normale verticale (calcul sur un patch horizontal appliqué au centre de chaque maille)
			rechercher les modifs  : //MODIF dec 2005



cc masques_ciel.c  pers_util.o  solutile.o  geomutile.o  face_op_face.o  solaire.o  poly_op_poly.o traite.o  lib_solene_94.o -o masques_ciel -lm


 POUR UN FICHIER .cir, determine, au centre de gravite de chaque contour, l'energie recue du ciel,
 en tenant compte des masques eventuels 
 en tenant compte de plusieurs luminances de ciel
	

 ET EN TENANT COMPTE DES TRANSMISSIONS LUMINEUSES DES VITRAGES TRAVERSES 
 ATTENTION les valeurs de transmission sont affectees aux contours                            
 mais on sait que le matériau est une propriété de la face et non pas du contour
 on peut donc lire les .val de transmission en tenant compte seulement 
 de la valeur de transmission du dernier contour de la face et l'attribuer à la face

ATTENTION les faces vitres sont definies deux fois (chacun avec sa transparence);                  
   une fois avec la normale dans un sens ,une autre avec la normale dans le sens opposé

   Pour les arbres, la face est définie une seule fois; leur mettre les normales habituelle 
   et leur attribuer la transmission de l'arbre

     			    // attention, on conserve les faces transparentes qui regardent vers le ciel
					// donc on inverse la normale des faces transparentes
					// seulement pour le test de visibilité

// ATTENTION  modif de mars 2004:
// Ne tient pas compte des faces situées dans le même plan que celui du contour de la face en traitement
// donc attention, si des faces intérieures non visibles (mitoyennes), alors il faut, pour avoir un résultat bon su ces faces (cad soleil =0)
// fournir un masque 
// qui contiennent 2 fois les faces du masque, avec les normales dans le bon sens et inversées.

*/

#include<solene.h>

//Declaration Functions
float	calcul_eclair_sur_plan_horizontal();
void	calcul_min_max();
void	imprime_en_tete_face();
void	inverse_normale();
void    lect_fic_val_pour_face();

void	masque_ciel();
void	met_extension();
void	ne_voit_rien();
void	patch_gravite();
int		test_si_patch_vu();
void	usage_masques_ciel();

//GLOBAL
struct modelisation_face *fac0 ;	/* geometrie a evaluer */
struct modelisation_face *fac1;		/* geometrie des masques */
struct modelisation_face *fac2;		/* geometrie du ciel */

#define HMAX 48  // nb max de fichiers Radiance ou luminance de ciel

FILE *pval[HMAX];

extern int option_calcul_z;   	/* option CALCUL du Z ds FACE-OP-FACE 1=oui */
                              	/* utilise ds singul.c epure_polygone */
			      	/* et dans face_op_face.c             */
extern double coef_discol;

int im,nbre_patch_vu,nb_pas;
float *valeur_calcul;
float *ecl_sur_plan_horizontal;
float *val;
double vnf[3];   		/* normale a la face en traitement */
double tgvis,covis,sinvis;
double *xg_patch,*yg_patch,*zg_patch,*angl_solid;
float *val_min,*val_max;
int		nb_transmission, nb_transmission_max;

 double	 epsiloneN;  // parametre de coplanaire
 int	 facteurD;   // parametre de coplanaire


/*_________________________________________________________________*/
main(argc,argv)           /* MASQUE  pour un FICHIER DE FACES */
int argc;char **argv;
	{
	int i,j,noc,nbfac0,nbfac1,nbfac2;
	int nomax0,nomax1,nomax2,nofac,nbcont;
 	double ang;
 	char nom_in[256],nom_masc[256],nom_val[256],nom_ciel[256],nom_lumin[256],nom_out[256],nom_angl_solid[256];
	char buf[256];
	char *s_dir,c;
 	FILE *pfic,*pficface;
 	struct contour *pcont;
 	struct circuit *pcir;
 	int *vis;
	double englob[10],mini,maxi;
	int indice;

	double *trans_masc;


	int hh1,hh2,pas,minute,temps;
	char extension[16];

//MODIF dec 2005
	int option; //0 (défaut, calcul suivant la normale de la maille)
				//1 (calcul suivant normale verticale)
//

	printf("Fonction Solene: masques_ciel_lum\n\n");
/* initialisation */
   	singularite=0; non_singularite=0; nb_etat=0;
   	pb_masque=0; colle_max=coef_discol*DISCOL;
   	pi=4*atan(1.);

   // initialisation pour coplanérité
   epsiloneN = 0.0001;
   facteurD= 1000;


	nb_transmission_max = 0;

	s_dir=(char *)getenv("PWD");

//MODIF dec 2005
//if(argc != 13) usage_masques_ciel();
if(argc != 13 && argc != 14) usage_masques_ciel();

	printf("Fonction Solene: masques_ciel_lum\n\n");
 
/* lecture parametres commande */

	compose_nom_complet(nom_in,s_dir,argv[1],"cir");
    printf("\n  geometrie a traiter : %s \n", nom_in);

    compose_nom_complet(nom_masc,s_dir,argv[2],"cir");  
    printf("  geometrie masque : %s \n", nom_masc);

    compose_nom_complet(nom_val,s_dir,argv[3],"val");  
    printf("avec fichier val de transmission : %s \n",nom_val);

	compose_nom_complet(nom_ciel,s_dir,argv[4],"cir");
    printf("  geometrie de ciel : %s \n", nom_ciel);

	compose_nom_complet(nom_angl_solid,s_dir,argv[5],"val");  
    printf("  angles solides : %s \n", nom_angl_solid);

	//  heure debut et fin ; pas

    sscanf(argv[6],"%d%c%d",&hh1,&c,&minute);
    printf("evalue de  %dH%d",hh1,minute);
    hh1=hh1*60+minute;
    sscanf(argv[7],"%d%c%d",&hh2,&c,&minute); 
    printf(" a  %dH%d",hh2,minute);
    hh2=hh2*60+minute;
    sscanf(argv[8],"%d%c%d",&pas,&c,&minute);
    printf(" par pas de  %dH%d\n",pas,minute);
    pas=pas*60+minute;

    // calcul du nbre de pas
    nb_pas=1;
    i=hh1;
    while(i<hh2)
    { nb_pas++;
      i=i+pas;
    }
    printf("nb de pas %d\n",nb_pas);

   if(nb_pas > HMAX)
	{ printf("trop de fichiers en sortie max %d\n",HMAX);
	  exit(0);
	}


    sscanf(argv[10],"%lf",&ang);
    if(ang<0 || ang>89.99 || ang<15) 
	   	{ 	
		  printf("   *** 15 < angle_vision < 89.99 ***\n");
	      exit(0);
	   	}
    ang=pi*ang/180.;
    tgvis=tan(ang); covis=cos(ang); sinvis=sin(ang);

    sscanf(argv[11],"%d",&nb_transmission);
	if(nb_transmission <0)
		printf("Toutes les transmissions sont considérées\n");
	else 
		printf("Seules sont considérées les énergies transmises après %d transmissions\n",nb_transmission);

    	im=0;   /* Pour IMPRESSION mettre  im = 1 */

//MODIF dec 2005
// lecture de l'option
	option=0;
	if(argc==14)
	{ sscanf(argv[13],"%d",&option);
	  if(option)printf("\nCalcul en considérant que la normale des mailles est verticale\n");
	  else printf("\nCalcul en considérant la normale des mailles \n");
	}
//

/* LECTURE DES FICHIERS */
/*** Lecture du fichier CIEL - indice 2 ***/
	if ((pfic=fopen(nom_ciel,"r"))==NULL)
      { 
		printf("\n  impossible ouvrir %s\n\n", nom_ciel); 
		exit(0);
      }
       	lit_en_tete(pfic,&nbfac2, &nomax2, englob);
       	fac2=alloue_face(nbfac2, 1000);
       	lit_fic_cir3d(pfic, nbfac2, fac2);
       	fclose(pfic);

	/* calcul centre gravite patch de ciel */
	
  	xg_patch=alloue_double(nbfac2,1);
  	yg_patch=alloue_double(nbfac2,2);
  	zg_patch=alloue_double(nbfac2,3);
  	patch_gravite(nbfac2);
    desalloue_fface(fac2,nbfac2);


/*** Lecture des nb_pas LUMINANCES ou radiances de CIEL - indice _2 */
/* attention peut_etre fonction du temps nb_pas*/

/* alloue nbfac2*nb_pas valeurs de luminance */
       	val=alloue_float(nbfac2*nb_pas,4);

       /* le 1er fic de luminance stocke de 0 a nbfac2-1 */
       /* le 2eme fic de luminance stocke de nbfac2 a ... ; et etc */
        indice=0;
		temps=hh1;
        for(i=0;i<nb_pas;i++)
         {
			// appel extension _..H..
			met_extension(temps,extension);
			// ouvre val resultat en Output
			sprintf(buf,"%s%s",argv[9],extension);
			compose_nom_complet(nom_lumin,s_dir,buf,"val");
            printf("  luminances ou radiances de ciel : %s \n", nom_lumin);
			if ((pfic=fopen(nom_lumin,"r"))==NULL)
				{ 
					printf("\n  impossible ouvrir %s\n\n", nom_lumin); 
					exit(0);
				}
			fscanf(pfic,"%d %d %lf %lf",&nbfac2,&nomax2,&mini,&maxi);
			for(j=0;j<nbfac2;j++)
				{ fscanf(pfic,"\n%c%d%d\n%f\n",&c,&nofac,&nbcont,val+indice+j);
				}
			fclose(pfic);
			indice=indice+nbfac2;
			temps+=pas;
         }

/* Lecture des ANGLES_SOLIDES CIEL des elements de ciel - indice _2 */
	if ((pfic=fopen(nom_angl_solid,"r"))==NULL)
      { 
		printf("\n  impossible ouvrir %s\n\n", nom_angl_solid); 
		exit(0);
      }
	fscanf(pfic,"%d %d %lf %lf",&nbfac2,&i,&mini,&maxi);
    angl_solid=alloue_double(nbfac2,5);
	for(i=0;i<nbfac2;i++)
		{
			fscanf(pfic,"\n%c%d%d\n%lf\n",&c,&nofac,&nbcont,angl_solid+i);
		}
    fclose(pfic);


/*** Lecture en tete de la GEOMETRIE a traiter ***/

		//changement

       	if ((pficface=fopen(nom_in,"r"))==NULL)
        { 
			printf("\n impossible ouvrir %s\n\n", nom_in); 
			exit(0);
        }
       	lit_en_tete(pficface,&nbfac0, &nomax0, englob);
       	printf("  %d faces a traiter", nbfac0);

/*** Lecture de GEOMETRIE MASQUE - indice 1 ***/
       	if ((pfic = fopen(nom_masc,"r"))==NULL)
        { 
			printf("\n impossible ouvrir %s\n", nom_masc); 
			exit(0);
        }
       	lit_en_tete(pfic,&nbfac1, &nomax1, englob);
       	printf(" avec masque de %d faces\n\n", nbfac1);
       	fac1=alloue_face(nbfac1, 34);
      	lit_fic_cir3d(pfic, nbfac1, fac1);
      	fclose(pfic);

	   // lit fichier val de transmission pour la face
	   // prend la valeur sur le dernier contour de chaque face et stocke
	   	if((pfic=fopen(nom_val,"r"))==NULL)
		{
         printf("\n impossible ouvrir %s\n",nom_val); 
		 exit(0);
  		}
        trans_masc = alloue_double(nbfac1,2345);
	    lect_fic_val_pour_face(pfic,trans_masc);
	    fclose(pfic);

        vis=alloue_int(nbfac1,1);


/*** OPEN LES FICHIERS VAL RESULTATS ***/
/* 0 a nbpas-1 : eclairement */

        indice=0;
	    temps=hh1;

	                   /** eclairement **/
	    mini=0; maxi=0;
        for(i=0;i<nb_pas;i++)
         {
			// appel extension _..H..
			met_extension(temps,extension);
			// ouvre val resultat en Output
			sprintf(buf,"%s%s",argv[12],extension);
			compose_nom_complet(nom_out,s_dir,buf,"val");
            printf("  descripteur a creer : %s \n", nom_out);
		    if ((pval[indice]=fopen(nom_out,"w"))==NULL)
            { 
			 printf("\n  impossible ouvrir %s\n\n", nom_out); 
			 exit(0);
            }
		   fprintf (pval[indice],"%5d %5d %10.2f %10.2f\n", nbfac0, nomax0, mini, maxi);   
    	   indice++;
		   temps+=pas;
         }


/*** Alloue les valeurs contenant les nbpas RESULTATS ***/
  	valeur_calcul=alloue_float(nb_pas,1);

/*** Alloue les valeurs contenant les nbpas val_min val_max ***/
/* 0 a nbpas-1 : eclairement */
        indice=nb_pas; 
  		val_min=alloue_float(indice,1);
  		val_max=alloue_float(indice,1);
        for(i=0;i<indice;i++) 
			{ val_min[i]=1000000; val_max[i]=-val_min[i];
			}

/*** EVALUE a chaque pas L'eclairement recu sur Un plan HORIZONTAL ***/
/* stocke les nb_pas valeurs */
       	ecl_sur_plan_horizontal=alloue_float(nb_pas,4);

        indice=0;
        for(i=0;i<nb_pas;i++)
         { ecl_sur_plan_horizontal[i]=calcul_eclair_sur_plan_horizontal(indice,nbfac2);
	       indice+=nbfac2;
         }

  
/* observateur regarde vers le haut a verticale de oeil */
        obs.x=0; obs.y=0; obs.z=-1; 


/* TRAITEMENT POUR CHAQUE CONTOUR DE CHAQUE FACE DE NOM_IN */
	printf("Traitement en cours ...\n");


  /* alloue 1 seule face */   
       	fac0=alloue_face(1,1000);

        for(i=0;i<nbfac0;i++)
           	{    /* lit la face */    	
                  lit_fic_cir3d(pficface, 1, fac0);

				noc=0;
				vnf[0]=(fac0)->vnorm[0]; 
				vnf[1]=(fac0)->vnorm[1]; 
				vnf[2]=(fac0)->vnorm[2]; 
//MODIF dec 2005
	if(option)
	{ vnf[0]= 0; vnf[1]=0; vnf[2]=1;
	}
//

				/** imprime dans .VAL pour chaque face **/
				imprime_en_tete_face(fac0);

             	pcont=(fac0)->debut_projete; 
				while(pcont)	   
               		{ 
						pcir=pcont->debut_support;
		                centre_de_gravite(pcir, &obs.xo, &obs.yo, &obs.zo);
                 		noc++;

						//printf("FACE %d  Contour %d\n",(fac0)->nofac_fichier,noc); 
                        //printf("   %lf %lf %lf\n",obs.xo,obs.yo,obs.zo);
			// traite la coplanerité (utile quand un masque n'appartenent pas à la scene traitée 
			//  est coplanaire avec le contour en traitement, donc le cache)
			// DANS LE CAS PRESENT, comme les vitrages sont doublés alors cela pose un pb pour les
			// parois vitrées qui sont doublées mais doivent être traitées
			// La solution serait de renvoyer le numero de face et verifier si un masque ou un vitrage
			// PÖUR LE MOMENT, COURCIRCUITE LA FONCTION

            //avec_meme_normale = 0;
			//if(traite_coplanerite(obs.xo,obs.yo,obs.zo,fac0,fac1,nbfac1,avec_meme_normale))
			//     {  
					//l'observateur est masqué totalement
			//		printf("   CACHE \n");
			//      ne_voit_rien(nbfac2;
				
		    //     }
		    //else 

		                  {
 						     /**APPEL FONCTION DE CALCUL */ 
							 //printf("FACE %d  Contour %d\n",(fac0)->nofac_fichier,noc);

                		     masque_ciel(nbfac1,nbfac2,fac0,vis,trans_masc);

						     //printf("  nombre de patches de ciel vus %d\n",nbre_patch_vu);
						  }
                 		pcont=pcont->suc;

                 		if(im)
                  		{ 
							printf("  FACE %d  Contour %d\n",(fac0)->nofac_fichier,noc);
                    		printf("  %lf %lf %lf\n",obs.xo,obs.yo,obs.zo);
							printf("  nombre de patches de ciel vus %d\n",nbre_patch_vu);
						} 
               		}
                 desalloue_contour_face(fac0); 
           	}
		desalloue_double(xg_patch);
		desalloue_double(yg_patch);
		desalloue_double(zg_patch);
	    desalloue_int(vis); desalloue_float(val); desalloue_double(angl_solid);
       	desalloue_fface(fac0,1);
       	desalloue_fface(fac1,nbfac1);
    	fclose(pficface);

   /* reecrit les min max pour ... */
        indice=0;
	                   /** eclairement **/
        for(i=0;i<nb_pas;i++)
         {
			rewind(pval[indice]);
			fprintf (pval[indice],"%5d %5d %10.2f %10.2f\n", nbfac0, nomax0, val_min[i], val_max[i]);
			fclose(pval[indice]);
    		indice++;
         }


     desalloue_float(val_min);
	 desalloue_float(val_max);
     desalloue_double(valeur_calcul);
	 desalloue_double(trans_masc);
     desalloue_float(ecl_sur_plan_horizontal);

	creer_OK_Solene();

	printf("nb de transmissions max enregistrée pour un patch : %d\n",nb_transmission_max);
	printf("\n\nFin du Traitement: masques_ciel_lum\n");
	exit(0);
	}


/*_________________________________________________________________*/
void masque_ciel(nbfac1,nbfac2,fac0,vis,trans_masc)
int nbfac1,nbfac2,*vis;
struct modelisation_face *fac0;
double *trans_masc;
	{
	int i,kj,vu,indice;
	double cos_ang_inc, xyz[3], ang_inc;
	double valeur_transmission;
	int avec_meme_normale;


   avec_meme_normale = 0; // quelque soit le sens de la normale
  /* TRANSFORMATION fichier "masque" et COUPE PYRAMIDE , si vu */
       tranfo();
       for(i=0;i<nbfac1;i++)
		{ //printf(" Examine avec face %d\n",(fac1+i)->nofac_fichier);
		  //if((fac1+i)->nofac_fichier != fac0->noface_fichier)
		  if(!(coplanaire(fac0,1,fac1+i,1,epsiloneN, 0,facteurD)))

              { //printf(" qui ne sont pas coplanaire\n");
			    // attention, on conserve les faces transparentes qui regardent vers le ciel
			    // donc on inverse la normale des faces transaparentes
			    // seulement pour le test de visibilité
			    if(trans_masc[i])
				{ inverse_normale(fac1+i);
				}
				
				if(visible_pers(fac1+i,1))
                { 
					 //printf(" avec face Visible  %d\n",(fac1+i)->nofac_fichier);
					 vis[i]=1;
					 if(trans_masc[i])
					 { inverse_normale(fac1+i);
					 }


                     tran_face(fac1+i,1,fac1+i,0);
                     tran_normale((fac1+i)->vnorm);
                     if((fac1+i)->debut_dessin) 
                     { calcul_d_du_plan(fac1+i,0);
                       face_dans_vision(fac1+i,0);
					 }
                 }
                 else
				 { vis[i]=0;
				   if(trans_masc[i])
				   { inverse_normale(fac1+i);
				   }
				 }
               }
             else vis[i]=0;
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
	   		     //printf("       avec face Visible  %d\n",(fac1+i)->nofac_fichier);
                 //liste_face(fac1+i,0);

               }
          }
  // ON NE FAIT PAS DE VU/CACHE

/** ANALYSE DES PATCHES DE CIEL **/
	for(kj=0;kj<nb_pas;kj++)
 	 { 
  	   valeur_calcul[kj]=0.0;
     }
	nbre_patch_vu=0;
           /* considere chaque patch avec sa luminance a chaque pas (nb-pas) */
  	for(i=0;i<nbfac2;i++)
      {  
		xyz[0]=xg_patch[i], xyz[1]=yg_patch[i], xyz[2]=zg_patch[i];
		cos_ang_inc=vincid(vnf,xyz,&ang_inc);
		if (cos_ang_inc>0)
		   { //printf("Voit patch ciel %d de normale %f5.1 %f5.1 %f5.1\n", i,xyz[0],xyz[1],xyz[2]); 
			 vu = test_si_patch_vu(nbfac1,xg_patch[i],yg_patch[i],zg_patch[i],&valeur_transmission,trans_masc);
		     nbre_patch_vu+=vu;
		     indice=0;
             for(kj=0;kj<nb_pas;kj++)
 		      {
			
       			if(vu)
				{ //printf("val transmission = %f\n",valeur_transmission);
					valeur_calcul[kj]+= val[i+indice]*cos_ang_inc*angl_solid[i] * valeur_transmission;  
				}
 			    indice+=nbfac2;
 		      }
		   }
	/*	printf("\n%d  %lf  a=%lf",i,cos_ang_inc,a);*/
      }

/* calcul des min_max pour la face en traitement */
   for(kj=0;kj<nb_pas;kj++)
    { 
          calcul_min_max(valeur_calcul[kj],val_min+kj,val_max+kj);
    }

/* ECRITURE Des FICHIERS .val */

        indice=0;
	                   /** eclairement **/
        for(i=0;i<nb_pas;i++)
         {
	      fprintf (pval[indice],"%10.2f\n",  valeur_calcul[i]);   
    	  indice++;
         }

/* reinverse la normale et desallocation face->dessin */  
     for(i=0;i<nbfac1;i++)
	  { if(vis[i])
              { tran_normale_inverse((fac1+i)->vnorm);
                vis[i]=0;
              }
        if((fac1+i)->debut_dessin)
              { desalloue_chaine_contour((fac1+i)->debut_dessin);
                (fac1+i)->debut_dessin=NULL;
              }

       }
}
/*-----------------------------------------------------------------------------*/
int test_si_patch_vu(nbfac1,xg,yg,zg,valeur_transmission,trans_masc)
int nbfac1;
double xg,yg,zg;
double *valeur_transmission, *trans_masc;
{
 int in,ij;
 double xyz[3];
 double xp, yp, zp;
 int nbt;

 // ne retient, si nbt_transmission>=0, que les énergies transmises après nbt_transmission
	*valeur_transmission = 1;
	nbt = 0;

    tranp(xg,yg,zg,xyz,xyz+1,xyz+2);
/* coupe par pyramide : retient ou non le point */

    if(xyz[2]<0 && fabs(xyz[0]/xyz[2])<tgvis && fabs(xyz[1]/xyz[2])<tgvis) 
     { 
/* met en pers */
        xp=-xyz[0]/xyz[2];
        yp=-xyz[1]/xyz[2];
        zp=0;
        normalise_point(xp,yp,zp,&xp,&yp,&zp);
		//printf(" le centre du patch en pers norm %f10.5 %f10.5 %f10.5\n",xp,yp,zp);
        /* test si dans masque */
		// ATTENTION la face peut être transparente
		// l'objet à voir (patch de ciel) est toujours à l'infini donc tout masque empêche de voir
        in=0;
        for(ij=0;ij<nbfac1;ij++)
          {  
			if((fac1+ij)->debut_dessin)
              { 
				//printf("  cherche masque avec face masque %d\n",ij+1 );
				if(point_dans_face(xp,yp,fac1+ij,0))
				{ 
				    //printf("     masque\n",ij );
				
					if(trans_masc[ij] == 0)
					{ // dans face opaque 
				      //printf("           par opaque\n",ij );
					  return(0);
					}
					else
					{ // dans face transparente
				      // donc transmission
					  //printf("           par vitre\n",ij );

					  *valeur_transmission =  *valeur_transmission * trans_masc[ij];
					  nbt++;

					  //printf("valeur_transmission ( %f) = %f\n",trans_masc[ij],*valeur_transmission);
					  // mais on continue jusqu'a trouver éventuellement un masque
					}
					
				}
              }
          }
                /*printf("xp= %lf yp= %lf in= %d\n",xp,yp,in);*/
		// ne retient que l'énergie correspondant au nb de transmission demandé
		if( nbt > nb_transmission_max) nb_transmission_max = nbt;
		if( nb_transmission < 0) return(1);
		if( nbt != nb_transmission) return(0);
		else return(1);
	 }
	// si le patch n'est pas dans le champ de vision, il n'est pas vu
 
	return(0);
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
		        centre_de_gravite(pcir, xg_patch+i, yg_patch+i, zg_patch+i);

             /*	printf ("\n%d  %lf %lf %lf",i, xg_patch[i],yg_patch[i],zg_patch[i]); 
      	printf ("\n%d  %lf",i, 180/pi*acos(zg_patch[i]/sqrt(xg_patch[i]*xg_patch[i]+yg_patch[i]*yg_patch[i]+zg_patch[i]*zg_patch[i]))); */
     		}
	}

/*_________________________________________________________________*/

float calcul_eclair_sur_plan_horizontal(indice,nbfac2)
int indice,nbfac2;
{

float ecl_sur_plan_horizontal;
int i,nbre_patch_vu;
double xyz[3],ang_inc,cos_ang_inc;

	nbre_patch_vu=0;
        ecl_sur_plan_horizontal=0;
        vnf[0]=0; vnf[1]=0; vnf[2]=1;

           /* considere chaque patch avec sa luminance a chaque pas (nb-pas) */
  	for(i=0;i<nbfac2;i++)
    		{  
		xyz[0]=xg_patch[i], xyz[1]=yg_patch[i], xyz[2]=zg_patch[i];
		cos_ang_inc=vincid(vnf,xyz,&ang_inc);
		if (cos_ang_inc>0)
		   {   
                          /*sans tenir compte du masque */
			ecl_sur_plan_horizontal+=val[i+indice]*cos_ang_inc*angl_solid[i];
			nbre_patch_vu++;
		   }
    		}
	printf("Nbre patch vus %d  ecl_horizontal %f\n",nbre_patch_vu,ecl_sur_plan_horizontal);

return(ecl_sur_plan_horizontal);
}

/*_________________________________________________________________*/

void calcul_min_max(valeur,val_min,val_max)
float valeur;
float *val_min,*val_max;
{ if(valeur<*val_min) *val_min=valeur;
  if(valeur>*val_max) *val_max=valeur;
}

/*_________________________________________________________________*/
void imprime_en_tete_face(fac0)
struct modelisation_face *fac0;
{
  int i,indice;

   indice=0;
	                   /** eclairement **/
    for(i=0;i<nb_pas;i++)
        {
			fprintf (pval[indice],"f%d %d\n",(fac0) ->nofac_fichier,nb_contour_face(fac0,1));   
    	    indice++;
        }

}
/*_________________________________________________________________*/
void ne_voit_rien(nbfac2)
int nbfac2;
{
 int kj,i,indice;

 for(kj=0;kj<nb_pas;kj++)
   { 
  	 valeur_calcul[kj]=0.0;
   }
 nbre_patch_vu=0;

/* ECRITURE Des FICHIERS .val */

 indice=0;
	                   /** eclairement **/
 for(i=0;i<nb_pas;i++)
   {
	 fprintf (pval[indice],"%10.2f\n",  valeur_calcul[i]);   
     indice++;
   }
		
}

/*_________________________________________________________________*/
/* Lecture d'un fichier  (.val) et remplissage tableau 'valeur' pour la face */
void lect_fic_val_pour_face(pfic, valeur)
FILE *pfic;
double *valeur;
{ double val_min;
  double val_max;
  int num_cont, num_face;
  int nbfac,nomax;
  int nofac, nbcont_face;
  char c;

  fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);
  for(num_face=0;num_face<nbfac;num_face++)
	{
	fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont_face);
	for(num_cont=0;num_cont<nbcont_face;num_cont++)	
		{	// ne stoke que la valeur du dernier contour pour la face
		fscanf(pfic,"%lf\n",valeur+num_face);
		}
	}
}

/*_________________________________________________________________*/
void inverse_normale(fac0)
struct modelisation_face *fac0;
{
		fac0->vnorm[0]= -fac0->vnorm[0]; 
	    fac0->vnorm[1]= -fac0->vnorm[1];
		fac0->vnorm[2]= -fac0->vnorm[2];
}

//_____________________________________________________
void met_extension (temps,extension)
int temps;
char *extension;
{
	float xh_heure;
	int h_heure, m_minute;
		xh_heure= (float)temps/60;
		h_heure= (int) xh_heure;
		m_minute= temps-h_heure*60;
		//printf(" heure_minute %d H %d\n",h_heure,m_minute);
		//construit extension pour fichier val heure
		if(h_heure>=10 && m_minute >=10)
		{
		 sprintf(extension,"_%dH%d",h_heure,m_minute);
		}
		else if(h_heure>=10 && m_minute <10)
		{
		 sprintf(extension,"_%dH0%d",h_heure,m_minute);
		} 
		else if(h_heure<10 && m_minute >=10)
		{
	     sprintf(extension,"_0%dH%d",h_heure,m_minute);
		} 
		else if(h_heure <10 && m_minute <10)
		{
		 sprintf(extension,"_0%dH0%d",h_heure,m_minute);
		} 
		//printf("extension %s\n",extension);
}

/*_________________________________________________________________*/
/* Format de la fonction masques_ciel */
void usage_masques_ciel()
{
 printf("\n  masques_ciel_lum   \n\n");
 printf("\n      la fonction a comme parametre ENTREE :\n\n");
 printf("\t geometrie_a_simuler_in(.cir)\n"); 
 printf("\t geometrie_masque_in(.cir)\n");
 printf("\t transmission_masque_in(.val)\n");
 printf("\t geometrie_de_ciel_in(.cir)\n"); 
 printf("\t angle_solide_de_ciel_in(.val)\n");
 
 printf("\t hh1:mn1\n");
 printf("\t hh2:mn2\n");
 printf("\t pas(hh:mn)\n");

 printf("\t NOM generique des fichiers de luminance ou radiance de ciel (.val)\n");
 printf("\t angle_vision\n"); 
 printf("\t nb de transmissions a considérer (<0 toutes, =N, seulement energie apres N transmissions\n");
   
 printf("\n           comme parametres en SORTIE, les descripteurs .val :\n\n");
   
 printf("\t NOM generique des fichiers énergie (solaire ou lumineuse) en provenance du ciel(.val)\n\n");
 //MODIF dec 2005
 printf("                  comme parametres OPTIONEL  :\n\n");
 printf("\t option_de_calcul\n\n");

 printf("\tNOTA:  option_de_calcul\n");
 printf("\t   0 défaut, calcul suivant la normale de la maille\n");
 printf("\t   1         calcul suivant normale verticale\n");
 //
 printf("\nNOTA: le programme ajoute une extension _hhHmm au nom de fichier correspondant à l'heure du calcul:\n");
 printf("\n\n		fichiers de luminance ou radiance de ciel  \n");
 printf("\n\n		fichiers énergie (solaire ou lumineuse) en provenance du ciel\n");
	exit(0);
}


