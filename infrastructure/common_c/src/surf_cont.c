/* surf_cont.c : CALCUL DES surfaces des CONTOURS				*/

/*______________________________________________________________________*/
// D. GROLEAU Avril 2003

#include <solene.h>

// FUNCTIONS
float cal_surf_contour();
void format_entree();
void surface_des_contours();
void usage();

// GLOBAL
FILE *fp1, *fp2;  

int nbfac1,nomax1;

float	*surf_cont;
struct modelisation_face *fac1;



/*____________________________________________*/

main (argc, argv)
int argc;
char *argv[];
{
	int		i,j,nop,nbcont;
	double min, max;
	double englob[10], surf_total_fic;

	char *s_dir;
	char entree[512], sortie[512];

	s_dir = (char *)getenv("PWD"); 
      	if(argc!=3) usage();
			printf("Fonction Solene : surf_cont\n\n");

/* OPEN du cir en in */
   	compose_nom_complet(entree,s_dir,argv[1],"cir");   /* entree .cir */
	if ((fp1 = fopen(entree,"r")) == NULL)
       		{ 
		printf("\n  le fichier %s n'existe pas !!\n\n", entree);
	 	exit(0);
       		}   	
	printf("\n  reading the file %s \n", entree);

/* OPEN du val en out */
	compose_nom_complet(sortie,s_dir,argv[2],"val");   /* sortie .val */
	if ((fp2 = fopen(sortie,"w")) == NULL)
		{
		printf("\n  impossible de creer %s !!\n\n", sortie);
		exit(0);
		}	
	printf("  writing the file %s \n\n", sortie);

	
/* LECTURE DE LA GEOMETRIE  .cir */
	lit_en_tete (fp1,&nbfac1,&nomax1,englob);
	printf ("   %d faces dans fichier %s\n", nbfac1,entree);
   /*	printf ("  numero maximum de faces = %d\n\n", numax);   */
	fac1 = alloue_face(nbfac1,1000);
	lit_fic_cir3d (fp1, nbfac1, fac1); 
   	fclose (fp1);



/* CALCUL DE L'AIRE DE CHAQUE contour */
	  nbc = nbcontours_total(fac1, nbfac1);
	  surf_cont = alloue_float(nbc, 1234);
	  surface_des_contours();
	  // calcule de la surface totale cumul�e des contours du fichier
	  surf_total_fic=0;
	  for (i=0;i<nbc; i++)
	  { surf_total_fic += surf_cont[i];
	  }
	  printf ("  aire totale de la g�om�trie %10.6f\n\n", surf_total_fic ); 


/* valeurs min et max */
	min = max = surf_cont[0];
        for (i = 0; i < nbc; i++) 
		{
		if (*(surf_cont+i) < min) min = *(surf_cont+i);
		if (*(surf_cont+i) > max) max = *(surf_cont+i);
		}

/* ECRITURE DU FICHIER .val */

	fprintf (fp2,"%d %d %.12f %.12f\n", nbfac1, nomax1, min, max);   
	nop=0;
	for (i = 0; i < nbfac1; i++) 
		{
			nbcont= nb_contour_face(fac1+i);
			fprintf (fp2,"f%d %d\n", i+1,nbcont );   
			for(j=0;j<nbcont;j++)
			{
			 fprintf (fp2,"      %.12f\n", *(surf_cont+nop)); 
			 nop++;
			}
		}
	desalloue_float(surf_cont);  

	creer_OK_Solene();
	exit(0);
	}

/*_____________________________________________________*/
void surface_des_contours()
{
 int i,nop;
 struct contour *pcont;
 struct circuit *pcir;
   nop=0;
	for(i=0;i<nbfac1;i++)
	{
		// pour calcule de la surface de chaque contour - celle des trous
                      obs.xo=0; obs.yo=0; obs.zo=0;
                      obs.x=(fac1+i)->vnorm[0];
                      obs.y=(fac1+i)->vnorm[1];
                      obs.z=(fac1+i)->vnorm[2];
                      tranfo();
					  tran_face(fac1+i,1,fac1+i,0); 

	   pcont=(fac1+i)->debut_dessin;
	   while(pcont)
	   {
		   pcir=pcont->debut_support;
		   surf_cont[nop] = cal_surf_contour(pcont);
		   nop++;
		   pcont=pcont->suc;
	   }
	 }
}

/*-----------------------------------------------------------------------------*/
float cal_surf_contour(pcont)
struct contour *pcont;
{ 
 float surf;
 struct circuit *pcir;

 surf=0;
	 pcir=pcont->debut_support;
	 surf+=(float)fabs(surface(pcir->nbp,pcir->x,pcir->y));
	 pcir=pcont->debut_interieur;
	 while(pcir)
	    {surf-=(float)fabs(surface(pcir->nbp,pcir->x,pcir->y));
	     pcir=pcir->suc;
	    }
  return(surf);
}


/*__________________________________________________________*/
/* Format de la fonction  */
void usage()
	{
	printf("\n   *surf_cont*  fichier_in (.cir) fichier_out (.val)");  
	printf("\n   evalue surface des contours \n\n");  	
	exit(0);                                         
	}


