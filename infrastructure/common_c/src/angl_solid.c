/* angl_solid.c : CALCUL DES ANGLES SOLIDES 				*/
/*	     	  valable uniquement pour les fichiers			*/
/*	     	  issus de GEODE (discretisation geodesique)		*/
/*______________________________________________________________________*/

/*
cc angl_solid.c geomutile.o solutile.o -o angl_solid lib_solene_94.o -lm
*/

#include <solene.h>
#include <stdlib.h>
#include <sys/stat.h>

FILE *fp1, *fp2;  

double nor[3], x[3], y[3], z[3],PI;
double *norm, *tang, *aire_face, aire;


/*____________________________________________*/

main (argc, argv)
int argc;
char *argv[];
	{
	int i, nbfac, numax;
	double min, max;
	double englob[10], aire_tot=0.;

	char *s_dir;
	char entree[512], sortie[512];
	struct modelisation_face *fac;

	s_dir = (char *)getenv("PWD");  
 
      	if(argc!=3) usage_solid();

   	compose_nom_complet(entree,s_dir,argv[1],"cir");   /* entree .cir */

/* OPEN du cir en in */
	if ((fp1 = fopen(entree,"r")) == NULL)
       		{ 
		printf("\n  le fichier %s n'existe pas !!\n\n", entree);
	 	exit(0);
       		}   	
	printf("\n  lecture du fichier %s \n", entree);


	compose_nom_complet(sortie,s_dir,argv[2],"val");   /* sortie .val */

/* OPEN du val en out */
	if ((fp2 = fopen(sortie,"w")) == NULL)
		{
		printf("\n  impossible de creer %s !!\n\n", sortie);
		exit(0);
		}	
	printf("  ecriture du fichier %s \n\n", sortie);


	
/* LECTURE DE LA GEOMETRIE DE CIEL .cir */
	lit_en_tete (fp1,&nbfac,&numax,englob);
	printf ("  calcul des angles solides des %d faces du fichier %s\n", nbfac,entree);
	if(nbfac!=4&&nbfac!=16&&nbfac!=64&&nbfac!=256&&nbfac!=1024&&nbfac!=4096&&nbfac!=16384) usage_solid();
   /*	printf ("  numero maximum de faces = %d\n\n", numax);   */
	fac = alloue_face(nbfac,1000);
	lit_fic_cir3d (fp1, nbfac, fac); 
   	fclose (fp1);



/* CALCUL DE L'AIRE DE CHAQUE FACETTE */
	norm = (double *) malloc(3*nbfac*sizeof(double)); 
	tang = (double *) malloc(6*nbfac*sizeof(double));  
 	aire_face = (double *) malloc(nbfac*sizeof(double)); 

	PI=4*atan(1.);
        for (i = 0; i < nbfac; i++) 
		{
	/*	printf ("\nface  %d", i+1);  */
		normales (fac+i);
		tangentes (fac+i);
		*(aire_face+i)=aire;
		}
	free(norm);
	free(tang);

/* valeurs min et max */
	min = max = aire_face[0];
        for (i = 0; i < nbfac; i++) 
		{
		aire_tot += *(aire_face+i);
		if (*(aire_face+i) < min) min = *(aire_face+i);
		if (*(aire_face+i) > max) max = *(aire_face+i);
		}
/*	printf ("  aire totale %10.6f\n\n", aire_tot);    */


/* ECRITURE DU FICHIER .val */
	fprintf (fp2,"%d %d %.12f %.12f\n", nbfac, nbfac, min, max);   
	for (i = 0; i < nbfac; i++) 
		{
		fprintf (fp2,"f%d 1\n", i+1);   
		fprintf (fp2,"      %.12f\n", *(aire_face+i));  
		}
	free(aire_face);  

	creer_OK_Solene();
	exit(0);
	}



/*_________________________________________________________________*/
/* normale au plan contenant 2 points et le centre de l'hemisphere */
/* n normales si face a n cotes */
int normales (fac1)
struct modelisation_face *fac1;
	{
 	struct contour *pcont;
 	struct circuit *pcir;
 	int i, n;
  	pcont=fac1->debut_projete; 
   	pcir=pcont->debut_support; 
   	n = pcir->nbp-1;
   	for(i=0; i< n; i++)
           	{ 
		x[0]=pcir->x[i], y[0]=pcir->y[i], z[0]=pcir->z[i];
		x[1]=pcir->x[i+1], y[1]=pcir->y[i+1], z[1]=pcir->z[i+1];
		x[2]=0, y[2]=0, z[2]=0;

		normale_avec_3pts(x,y,z,nor);
		*(norm+3*i)=nor[0], *(norm+3*i+1)=nor[1], *(norm+3*i+2)=nor[2];
	   	}
	}


/*__________________________________________________________*/
/* tangentes en un point du grand cercle, dans 2 directions */
/* 2*n tangentes si face a n cotes */
int tangentes (fac1)
struct modelisation_face *fac1;
	{
 	struct contour *pcont;
 	struct circuit *pcir;
 	int i, j, n;
	double scal;
  	pcont=fac1->debut_projete; 
   	pcir=pcont->debut_support; 
   	n = pcir->nbp-1;
	for(i=0; i< n; i++)
           	{
		x[0]=*(norm+3*i), y[0]=*(norm+3*i+1), z[0]=*(norm+3*i+2);	
		x[1]=pcir->x[i], y[1]=pcir->y[i], z[1]=pcir->z[i];
		x[2]=0, y[2]=0, z[2]=0;
		normale_avec_3pts(x,y,z,nor);
		*(tang+6*i)=nor[0], *(tang+6*i+1)=nor[1], *(tang+6*i+2)=nor[2];
		
		j=(i+n-1)%n;
		x[0]=-*(norm+3*j), y[0]=-*(norm+3*j+1), z[0]=-*(norm+3*j+2);
		normale_avec_3pts(x,y,z,nor);
		*(tang+6*i+3)=nor[0], *(tang+6*i+4)=nor[1], *(tang+6*i+5)=nor[2];
		}

/* Calcul des 3 angles du triangle spherique, puis de sa surface */ 
	aire=0.;	
	for(i=0; i< n; i++)
           	{	
		scal=0.;
		for(j=0; j< 3; j++)
			{
			scal +=(*(tang+6*i+j))*(*(tang+6*i+j+3));
			}
		aire += acos(scal);

	/*	printf ("\n%d  aire %10.5f", i, aire);  */
		}

	aire -= (n-2)*PI;
	}



/*__________________________________________________________*/
/* Format de la fonction SOLID */
int usage_solid()
	{
	printf("\n   *angl_solid*  fichier_in (.cir) fichier_out (.val)");  
	printf("\n   convient pour une discretisation geodesique uniquement !!\n\n");  	
	exit(0);                                         
	}


