 /* radiosite.c : ENERGIE ABSORBEE PAR LES PATCHES DE LA SCENE SELON 	*/
/*  		 LA METHODE DE RADIOSITE (RAFFINEMENT PROGRESSIF)	*/
/*		 AVEC ACCES SEQUENTIEL AUX FACTEURS DE FORME		*/
/*______________________________________________________________________*/

/*
cc radiosite.c  geomutile.o solutile.o lib_solene_94.o -o radiosite -lm
*/

// D. GROLEAU 8 juin 2005 ; modif test d'arret;  "s" n'est testé que si option "s" et non pas tout le temps avec s=0 
//			(on pouvait avoir des valeurs <0 dans le test somme_preced - somme, ce qui d'ailleurs n'est pas normal, mais peut-etre 
//           à cause des calculs, des fac de forme, des réciprocités des facteurs)



#include<solene.h>


// declare fonctions
void usage_rad();


void lecture_ff();
FILE *fp;


/*_________________________________________________________________*/
main(argc,argv)           
int argc;
char **argv;
	{
	int i,j,ii,iface,icont,k,im,nb_iter,*nb_cont_face,*numero_face;
	int nbfac,nofac,nomax,nbcont,num,nb_contour_total;
	double val_min,val_max,epsilon,somme,s_max,mini,maxi,somme_preced;
	double *M,*deltaM,*exit_init,*surf,*reflex,*Mprim,seuil,recipro,pourc;
	float *fform;
   	char nom_init[256],nom_reflex[256],nom_surf[256],nom_ff[256],
				nom_out[256],nom_out1[256],nom_out2[256];
	char *s_dir,c;
 	FILE *pfic,*pval,*pval1,*pval2;
	double dM,rien;
	int option_s;

	s_dir=(char *)getenv("PWD");
printf("%d arguments\n",argc);
   	if(argc<10) usage_rad();
 
/* lecture parametres commande */
       compose_nom_complet(nom_init,s_dir,argv[1],"val");
        printf("\n  exitances initiales : %s \n", nom_init);

	compose_nom_complet(nom_reflex,s_dir,argv[2],"val");
        printf("  coefficients de reflexion : %s \n", nom_reflex);

	compose_nom_complet(nom_ff,s_dir,argv[3],"fac");
        printf("  facteurs de forme : %s \n", nom_ff);

	compose_nom_complet(nom_surf,s_dir,argv[4],"val");
        printf("  surfaces : %s \n", nom_surf);

	compose_nom_complet(nom_out1,s_dir,argv[5],"val");
        printf("  exitances incidentes (W/m2): %s \n", nom_out1);

	compose_nom_complet(nom_out,s_dir,argv[6],"val");
        printf("  exitances reflechies (W/m2): %s \n", nom_out);

	compose_nom_complet(nom_out2,s_dir,argv[7],"val");
        printf("  exitances absorbees (W/m2): %s \n", nom_out2);

	pourc=0; seuil =0; option_s=0;
	sscanf(argv[8],"%c",&c); 
	 	if(c=='e') 
			{
			sscanf(argv[9],"%lf",&epsilon);
			nb_iter=1000000;
			printf("  epsilon : %lf\n\n",epsilon);
			}
		else if(c=='i') 
			{
			sscanf(argv[9],"%d",&nb_iter);
			epsilon=1e-6;
			printf("  nombre d'iterations : %d\n\n",nb_iter);
			}

		else if(c=='s') 
			{
			sscanf(argv[9],"%lf",&seuil);
			epsilon=1e-6;
			nb_iter=1000000;
			printf("  diff max entre 2 iterations : %f\n\n",seuil);
			option_s=1;
			}

		else if(c=='p') 
			{
			sscanf(argv[9],"%lf",&pourc);
			epsilon=1e-6;
			nb_iter=1000000;
			printf("  pourcentage de convergence : %.0lf %%\n\n",100-pourc);
			}

        im=0;	//=2 impression, =0 sans impression
    	if(argc==11)		/* impression ecran */
		{ 
		sscanf(argv[10],"%c",&c); 
	 	if(c=='p') im=1;
		if(c=='q') im=2;
		}


/* LECTURE D'UN FICHIER .val POUR NBRE DE CONTOURS TOTAL */
/* Lecture du fichier des exitances initiales */
	nb_contour_total=0;
	if ((pfic=fopen(nom_init,"r"))==NULL)
            	{ 
		printf("\n  impossible ouvrir %s\n\n", nom_init); 
		exit(0);
            	}
	fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);	
	numero_face=(int *)malloc(nbfac*sizeof(int));
	nb_cont_face=(int *)malloc(nbfac*sizeof(int));
	for(i=0;i<nbfac;i++)
		{
		fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont);
		numero_face[i]=nofac;
		nb_cont_face[i]=nbcont;
		nb_contour_total+=nbcont;
		for(k=0;k<nbcont;k++)
			fscanf(pfic,"%lf\n",&rien);
		}
	

	printf("nb_contour_total %d\n",nb_contour_total);	

/* allocation memoire */
	exit_init =(double *)malloc(nb_contour_total*sizeof(double));
	reflex =(double *)malloc(nb_contour_total*sizeof(double));
	surf =(double *)malloc(nb_contour_total*sizeof(double));
	M =(double *)malloc(nb_contour_total*sizeof(double));
    deltaM =(double *)malloc(nb_contour_total*sizeof(double));
   	fform =(float *) malloc(nb_contour_total*sizeof(float));  
	if(exit_init==NULL||reflex==NULL||surf==NULL||M==NULL||deltaM==NULL||fform==NULL)
	{ printf("\npb d'allocation (nbre de contours max ~ 2350000)\n");
	  exit(0);
	}
	else printf("\nok allocation\n");


/* LECTURE DES AUTRES FICHIERS */
/* Lecture du fichier des exitances initiales */
	rewind(pfic);
	fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);
	num=0;
	for(i=0;i<nbfac;i++)
		{
		fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont);
		for(k=0;k<nbcont;k++)	
			{	
			fscanf(pfic,"%lf\n",exit_init+num);
			num++;
			}
		}
    fclose(pfic);


/* Lecture des coefficients de reflexion */
	if ((pfic=fopen(nom_reflex,"r"))==NULL)
            	{ 
		printf("\n  impossible ouvrir %s\n\n", nom_reflex); 
		exit(0);
            	}
	fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);
	num=0;
	for(i=0;i<nbfac;i++)
		{
		fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont);
		for(k=0;k<nbcont;k++)
			{
			fscanf(pfic,"%lf\n",reflex+num);
			num++;
			}
		}
    fclose(pfic);

	
/* Lecture de la surface de chaque face */
	if ((pfic=fopen(nom_surf,"r"))==NULL)
            	{ 
		printf("\n  impossible ouvrir %s\n\n", nom_surf); 
		exit(0);
            	}
	fscanf(pfic,"%d %d %lf %lf",&nbfac,&nomax,&val_min,&val_max);
	num=0;
	for(i=0;i<nbfac;i++)
		{
		fscanf(pfic,"\n%c%d%d\n",&c,&nofac,&nbcont);
		for(k=0;k<nbcont;k++)
			{
			fscanf(pfic,"%lf\n",surf+num);
			num++;
			}
		}
    fclose(pfic);


/* Lecture des facteurs de forme */
	if ((fp=fopen(nom_ff,"rb"))==NULL)
            	{ 
		printf("\n  impossible ouvrir %s\n\n", nom_ff); 
		exit(0);
            	}


/* IMPRESSION */
	if(im==2)	{
		num=0;
		for(i=0;i<nbfac;i++)
			{
			for(k=0;k<nb_cont_face[i];k++)
				{
				printf("\nface %d contour %d : \n",numero_face[i],k+1);
				printf("  exit_init %.2lf\treflexion %.2lf\t   surface %.5lf\n",
							exit_init[num],reflex[num],surf[num]);
				num++;
				}
			}
		printf("\n\n");
		}


/* ECRITURE DE l'EN-TETE DU FICHIER RESULTAT INITIALES */
 	mini=maxi=0.0;
	if ((pval1 = fopen(nom_out1,"w"))==NULL)
           	{ 	
		printf("\n impossible ouvrir %s\n", nom_out1); 
		exit(0);
           	}
 	fprintf (pval1,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);

/* ECRITURE DE l'EN-TETE DU FICHIER RESULTAT REFLECHIES */
	if ((pval = fopen(nom_out,"w"))==NULL)
           	{ 	
		printf("\n impossible ouvrir %s\n", nom_out); 
		exit(0);
           	}
 	fprintf (pval,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);   
   	      	
/* ECRITURE DE l'EN-TETE DU FICHIER RESULTAT ABSORBEES */
	if ((pval2 = fopen(nom_out2,"w"))==NULL)
           	{ 	
		printf("\n impossible ouvrir %s\n", nom_out2); 
		exit(0);
           	}
 	fprintf (pval2,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);


/* RADIOSITE : METHODE A RAFFINEMENT PROGRESSIF */
	printf("\nTraitement en cours ...");
	printf("\n");

/* initialisations */
	for(i=0;i<nb_contour_total;i++)
	  {	M[i]=deltaM[i]=exit_init[i]*reflex[i];	
          }

	somme=0.0;
   	for(i=0;i<nb_contour_total;i++) 
      		somme+=deltaM[i]*surf[i];
   	printf("\nEnergie totale a distribuer (W): %.2lf\n",somme);
	somme_preced=somme;
	if (pourc) epsilon=somme*pourc/100;
	somme=2.*epsilon;


/* iterations */
	printf("\n");
	j=1;
	while(somme>epsilon && j<=nb_iter) 
   		{
   		printf("...iteration %d...\n",j); 

   		s_max=0.0;
   		num=0;
		for(i=0;i<nbfac;i++)
			{
			for(k=0;k<nb_cont_face[i];k++)
				{
				if(deltaM[num]*surf[num]>s_max)
         				{
         				s_max=deltaM[num]*surf[num];
					iface=i;     /* indice de face */
					icont=k+1;   /* numero de contour par face */
         				ii=num;	     /* numero de contour dans la liste */
         				}		 		    
				num++;
				}
			}

   		if(im) printf("\nFlux maxi (W) en provenance de la face %d, contour %d (no %d) : %5.2f\n",numero_face[iface],icont,ii+1,s_max);  

/* lecture des ff[ii] */
	/*	printf("ii = %d   nb_contour_total = %d\n",ii+1,nb_contour_total);  */
   		lecture_ff(fform,ii,nb_contour_total);   

   		num=0;
		if(im==2)
		  {
			for(i=0;i<nbfac;i++)
				{
				for(k=0;k<nb_cont_face[i];k++)
					{
	   printf("  FF f%dc%d -> f%dc%d = ",numero_face[iface],icont,numero_face[i],k+1);
					if(fform[num]==0) printf("0\n");
					else printf("%f\n",fform[num]);
					num++;
					}
				}
		   }


/* contribution de l'element ii a tous les autres */
   		for(i=0;i<nb_contour_total;i++) 
      			{
		/*	printf("%d %.5f\n",ii*nb_contour_total+i+1,fform[i]); */
			recipro=fform[i]*surf[ii]/surf[i];
			if(recipro>1.) 
			{ // recipro=1.; VOIR note algo eclairement, et note Fernand 24 avril 2002 
			  printf("recipro = %f\n",recipro);
			}
			dM=reflex[i]*deltaM[ii]*recipro;    
      			deltaM[i]+=dM;
      			M[i]+=dM;
      			if(im==2) printf("dM = %8.2f\tdeltaM[%2d] = %8.2f\tM[%2d] = %8.2f\n",
							dM,i+1,deltaM[i],i+1,M[i]);   
      			}
   		deltaM[ii]=0.0;

		somme=0.0;
   		for(i=0;i<nb_contour_total;i++) 
      			somme+=deltaM[i]*surf[i]; 
  
   		if(im) printf("energie non distribuee (W): %.2lf\n\n",somme);
// Modif GROLEAU 7 juin 2005; on ne fait le test que si option "s"
		if(option_s)
		{
		  printf("toto\n");
		  if(somme_preced-somme<seuil)
		  {printf("dif%f\n",somme_preced-somme);
			break;
		  }
		}
	
		somme_preced=somme;

	   	j++;
   	} //FIN DE WHILE

	printf("energie non distribuee (W): %.2lf\n\n",somme);
	free(deltaM);


/* EXPLOITATION DES RESULTATS M[i] */
	Mprim =(double *)malloc(nb_contour_total*sizeof(double));
	if(Mprim==NULL)
		{ printf("\npb d'allocation\n");
	  	exit(0);
		}

/* ecriture directe du fichier resultat */
/* mini et maxi */
	mini=maxi=M[0];
	for(i=0;i<nb_contour_total;i++)
		{
		if(M[i]<mini) mini=M[i];
		if(M[i]>maxi) maxi=M[i];
		}

	rewind(pval);
	fprintf (pval,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);   

	num=0;
	for(i=0;i<nbfac;i++)
		{
		fprintf (pval,"f%d %d\n",numero_face[i],nb_cont_face[i]);
		for(k=0;k<nb_cont_face[i];k++)
			{
			fprintf (pval,"    %8.3f\n",M[num]);
			num++;
			}
		}


/* valeur incidente sur chaque patch */
	for(i=0;i<nb_contour_total;i++) 
		{   	
 		Mprim[i]=M[i]/reflex[i];
		}

/* mini et maxi */
	mini=maxi=Mprim[0];
	for(i=0;i<nb_contour_total;i++)
		{
		if(Mprim[i]<mini) mini=Mprim[i];
		if(Mprim[i]>maxi) maxi=Mprim[i];
		}

	rewind(pval1);
	fprintf (pval1,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);   

	num=0;
	for(i=0;i<nbfac;i++)
		{
		fprintf (pval1,"f%d %d\n",numero_face[i],nb_cont_face[i]);
		for(k=0;k<nb_cont_face[i];k++)
			{
			fprintf (pval1,"    %8.3f\n",Mprim[num]);
			num++;
			}
		}


/* valeur absorbee par chaque patch */
	
	for(i=0;i<nb_contour_total;i++) 
		{   	
 		Mprim[i]=Mprim[i]-M[i];
		}

/* mini et maxi */
	mini=maxi=Mprim[0];
	for(i=0;i<nb_contour_total;i++)
		{
		if(Mprim[i]<mini) mini=Mprim[i];
		if(Mprim[i]>maxi) maxi=Mprim[i];
		} 

	rewind(pval2);
	fprintf (pval2,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);   

	num=0;
	for(i=0;i<nbfac;i++)
		{
		fprintf (pval2,"f%d %d\n",numero_face[i],nb_cont_face[i]);
		for(k=0;k<nb_cont_face[i];k++)
			{
			fprintf (pval2,"    %8.3f\n",Mprim[num]);
			num++;
			}
		}



/* ECRITURE DU FICHIER .val_energie */
/* mini et maxi */
/*	mini=maxi=absorb[0]*surf[0];
	for(i=0;i<nb_contour_total;i++)
		{
		absorb[i]=absorb[i]*surf[i];
		if(absorb[i]<mini) mini=absorb[i];
		if(absorb[i]>maxi) maxi=absorb[i];
		}

	rewind(pval1);
	fprintf (pval3,"%5d %5d %8.3lf %8.3lf\n", nbfac, nomax, mini, maxi);   

	num=0;
	for(i=0;i<nbfac;i++)
		{
		fprintf (pval3,"f%d %d\n",numero_face[i],nb_cont_face[i]);
		for(k=0;k<nb_cont_face[i];k++)
			{
			fprintf (pval3,"    %8.3f\n",absorb[num]);
			num++;
			}
		}
*/

	fclose(pval);	
	fclose(pval1);	
	fclose(pval2);	
     	fclose(fp);	
	
	free(surf);
	free(exit_init);
	free(reflex);
	free(fform);
	free(M);
	free(Mprim);

	printf("Fin radiosite\n\n");
creer_OK_Solene();
	exit(0);
	}

/*____________________________________________*/
/* Lecture des facteurs de forme dans fichier */
void lecture_ff(factf, ii, nb_contour_total)
float *factf;
int ii, nb_contour_total;
   {
   fseek(fp,(ii*nb_contour_total)*sizeof(float),SEEK_SET);
   fread(factf, sizeof(float), nb_contour_total, fp);
   }

/*_________________________________________________________________*/
/* Format de la fonction radiosite */
void usage_rad()
	{
  	printf("\n   format d'entree des parametres (option p : impression)\n\n");
  	printf("   *radiosite*  exitances_initiales(.val)  coef_reflexion(.val)  facteurs_forme(.fac)  surfaces(.val)  out_incident_W/m2(.val)  out_reflechi_W/m2(.val)  out_absorbe_W/m2(.val)  i entier/e decimal/p decimal/s decimal  [p(impression)]\n");

 printf("\n      la fonction a comme parametres en ENTREE :\n\n");
 printf("\t exitances_initiales(.val) sur la geometrie\n"); 
 printf("\t coef_reflexion(.val) des contours de la geometrie\n"); 
 printf("\t le fichier des facteurs de forme (.ff) de la geometrie\n"); 
 printf("\t les surfaces (.val) des contours de la geometrie\n"); 

  printf("\n           comme parametres en SORTIE, les descripteurs .val :\n\n");
   
 printf("\t eclairement (ou rayonnement) incident apres reflexion (.val)\n"); 
 printf("\t eclairement (ou rayonnement) reflechi apres reflexion (.val)\n"); 
 printf("\t eclairement (ou rayonnement) absorbe apres reflexion (.val)\n"); 

  printf("\n           comme parametres d'arret, soit :\n\n");
	printf("\ti suivi du nombre d'iterations maxi\n");
	printf("\te suivi de valeur d'energie restant a distribuer\n");
	printf("\tp suivi de valeur en %% d'energie restant a distribuer/premiere iteration\n");
	printf("\ts suivi de valeur de la difference maxi entre 2 iterations\n\n");


 	exit(0);
	}
