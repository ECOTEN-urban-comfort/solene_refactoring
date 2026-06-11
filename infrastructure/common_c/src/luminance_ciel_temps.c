// luminance_ciel_temps.c : DISTRIBUTION DES LUMINANCES DE CIEL SELON MODELE PEREZ	

// D. GROLEAU fev 2006 ; idem à luminance_ciel
//						 mais les fichiers de sortie sont indexés avec l'heure _--H-- au lieu d'un indice de 0 à n
// NOTA il devrait à terme remplacer luminance_ciel, à condition de modifier l'appel de la fonction et
//		 la récupération des fichiers résultats dans l'interface SOLENE 
                                           
//    3 types de sorties : 1 radiance spectre visible (W/m2)		
//     			   2 luminance (cd/m2 ou lm/m2/sr)		
//    			   3 radiance spectre total (W/m2)		

/* ATTENTION , considère que si epsilone=1 et delta=0.01,    */
/* le ciel est uniforme et donne 10000lux au sol             */
/* en 1ere approximation, cela revient à imposer à chaque    */
/* patch quelque soit sa surface une valeur de 3180cd/m2     */
/* _____________________________________________________________________*/

/*
cc luminance_ciel_temps.c geomutile.o solutile.o lib_solene_94.o solaire.o -o luminance_ciel_temps -lm
*/
#include <solene.h>
#include <luminance_ciel.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


//  PROTOTYPES DE FONCTIONS  

void centre_de_gravite();
int creer_OK_Solene();
int desalloue_double();  
void met_extension_heure();


// GLOBAL

FILE *fp1, *fp2;  
double rayon = 1.0;
int type_out;
double a[9], b[5][4];

// ATTENTION  ATTENTION
// ce programme fonctionnait sous visual 4
// a des resultats faux avec visual 5
// dépendant des impressions qu'on peut placer ici ou la pour le faire marcher

//????? bizarre apparemment pas de pb allocation
/*____________________________________________*/

main (argc, argv)
int argc;
char *argv[];
	{
// Dbt modif TL le 20070319
    char perezPath[1024];
// Fin modif TL le 20070319

	int i, j,kk,il,jl, interval,WHITEFF=179;
	int nbfac, numax;
	int hh1,hh2,pas,minute,nb_pas,temps,h_deb,m_deb;
	double xvar;
	double  Z;
	double  min, max,integ;
	double  *per_;
	double xg, yg, zg;
	double epsilon, delta, w=2; 	/* 2 coefficients du modele de Perez + hauteur d'eau */
	double englob[10];

	int    jd, mois, jour;
	double latit, longit; 
	double *lum;
	double sd, st, h, lumin, integr,lzenit,az;
	double De, Se, Dv, Sv,PI;

	char *s_dir,c;
	char entree[512], sortie[512];
	struct modelisation_face *face1;
	struct contour *pcont;
 	struct circuit *pcir;

	double xxxx;// bidon
	char buf[512],extension[16];


	s_dir = (char *)getenv("PWD");  

      	if(argc < 12) usage_ciel();
	printf("Fonction Solene : luminance_ciel_temps\n\n");

   	compose_nom_complet(entree,s_dir,argv[1],"cir"); 	/* fichier d'entree ciel.cir */

	sscanf (argv[2], "%lf", &latit);        		/* en degres */
	sscanf (argv[3], "%lf", &longit); 
      		/*     "     */
  	sscanf(argv[4],"%d%c%d",&jour,&c,&mois);
	//printf("jour mois %d %d\n",jour,mois);

         sscanf(argv[5],"%d%c%d",&hh1,&c,&minute);
         hh1=hh1*60+minute;
         sscanf(argv[6],"%d%c%d",&hh2,&c,&minute); 
         hh2=hh2*60+minute;
         sscanf(argv[7],"%d%c%d",&pas,&c,&minute);
         pas=pas*60+minute;
		 printf("hh1 hh2 pas %d %d %d\n",hh1,hh2,pas);

 /* calcul du nombre de pas */
   nb_pas=1;
   i=hh1;
   while(i<hh2)
    { nb_pas++;
      i=i+pas;
    }
	//printf("nb de pas %d\n",nb_pas);

  
/* Coefficients du type Perez epsilon et delta */   
	sscanf(argv[8], "%lf", &epsilon); 
	if(epsilon < 1 || epsilon > 12) usage_ciel();

 	sscanf(argv[9], "%lf", &delta);
	if(delta < 0.01 || delta > 0.6) usage_ciel();	

 	
/* Type de sortie */   	/* 1 : radiance spectre visible W/m2/sr */
						/* 2 : luminance cd/m2 ou lm/m2/sr */
						/* 3 : radiance solaire spectre total W/m2/sr */
	sscanf (argv[10], "%d", &type_out);         
	if(type_out<1||type_out>3) usage_ciel();

	

/* OPEN du cir en in */
	if ((fp1 = fopen(entree,"r")) == NULL)
       		{ 
		printf("\n  le fichier %s n'existe pas !!\n\n", entree);
	 	exit(0);
       		}   	
	printf("\n  lecture du fichier %s \n", entree);


/* LECTURE DE LA GEOMETRIE DE CIEL .cir */
	lit_en_tete (fp1,&nbfac,&numax,englob);
/*	printf ("  nombre de faces du fichier = %d\n", nbfac);
	printf ("  numero maximum de faces = %d\n", numax);
	for (i = 0; i < 10; i+=2)
		printf("%5.3f %5.3f\n", englob[i], englob[i+1]);
*/
	
	face1 = alloue_face(nbfac,1000);
	lit_fic_cir3d (fp1, nbfac, face1); 
	rayon=englob[1];
   	fclose (fp1);


/*  LECTURE DES FICHIERS DE VALEURS POUR LES MODELES PERRAUDEAU ET PEREZ  */
	/* intervalles du modele Perez */
	per_ = alloue_double(4*5*9,7685);  
 
	if (epsilon >= 1.000 && epsilon < 1.065) interval = 0;
	if (epsilon >= 1.065 && epsilon < 1.230) interval = 1;
	if (epsilon >= 1.230 && epsilon < 1.500) interval = 2;
	if (epsilon >= 1.500 && epsilon < 1.950) interval = 3;
	if (epsilon >= 1.950 && epsilon < 2.800) interval = 4;
	if (epsilon >= 2.800 && epsilon < 4.500) interval = 5;
	if (epsilon >= 4.500 && epsilon < 6.200) interval = 6;
	if (epsilon >= 6.200 && epsilon < 14.00) interval = 7;

// Dbt modif TL le 20070319
#ifdef LINUX
    sprintf(perezPath, "%s/%s", getenv("SOLENEHOME"), "data/perez.dat");
#else
    sprintf(perezPath, "%s\\%s", getenv("SOLENEHOME"), "data\\perez.dat");
#endif 
    lect_coef_per(perezPath, per_);
//	lect_coef_per("C:\\Solene\\Private\\Donnees\\perez.dat", per_); 
// Fin modif TL le 20070319

	//printf("epsilon delta %f %f\n",epsilon,delta);


	PI=4*atan(1.);
	/* caracteristiques du jour */
	jd = jdate(mois, jour);
	sd = declin (jd);

/* CALCUL DE LA LUMINANCE POUR CHAQUE FACE FONCTION DU TEMPS*/
/*__________________________________________________________*/
lum = alloue_double(nbfac,8974); 

temps=hh1;

for(kk=0;kk<nb_pas;kk++)
 {
/*  CALCULS a l'heure H  */
    xvar=temps/60.;
    h_deb= xvar;
    m_deb= temps-h_deb*60.;
    printf("a %d H %d\n",h_deb,m_deb);
    heure_et_minute_EN_heure_double(h_deb,m_deb,&st); 
	h = alt (sd, st,latit); 
	//printf("h= %f\n",h);

	Z = PI/2-h;
	az = 3*PI/2-azim (sd, st,latit);
    /*printf("\naz%6.2f",az*180/PI);*/

    /* calcul des valeurs de luminances */

	for(il = 0; il < 5; il++)
	{ 
	  for(jl = 0; jl < 4; jl++)
		{
		  b[il][jl] = *(per_ + 20*interval + 4*il + jl);
	      //printf("\n%d %d %10.6f", il, jl, b[il][jl]);  
		}
	}
	for(il = 0; il < 5; il++)
	{
	  a[il] = b[il][0] + b[il][1]*Z + delta*(b[il][2]+b[il][3]*Z);
	}
	if (interval == 0)
	{
	  a[2] = exp(pow(delta*(b[2][0] + b[2][1]*Z), b[2][2])) - b[2][3];
	  a[3] = -exp(delta*(b[3][0] + b[3][1]*Z)) + b[3][2] + delta*b[3][3];
	}

/* irradiance diffuse */
	printf("h %f\n",h);

	xxxx= airmass(h);
	//printf("xxxx %f\n",xxxx);
	De=delta*1367*exentr(jd)/xxxx;

/* irradiance directe */
	Se=De*(epsilon-1)*(1+1.041*Z*Z*Z);
/* illuminance diffuse */
	Dv=De*effic_lum_diffus(delta,Z,w,interval);
/* illuminance directe */
	Sv=Se*effic_lum_normal(delta,Z,w,interval);
	/*printf("De Se Dv Sv %f %f %f %f\n",De,Se,Dv,Sv);*/

 	integr=0;
	for (j = 0; j < nbfac; j++) 
	{

	 pcont=(face1+j)->debut_projete; 
   	 pcir=pcont->debut_support; 
	 centre_de_gravite(pcir, &xg, &yg, &zg);

     //printf ("\n%d   %5.3f %5.3f %5.3f", j+1, xg, yg, zg);
	 luminance (xg, yg, h, az, &lumin, &integr);
	 *(lum+j) = lumin;
	 //printf("lumin integr %f %f\n",lumin,integr);
	}

	/* integration et valeurs min et max */
	if(type_out==1)
		integr *= (2*PI/(nbfac*Dv))*WHITEFF; 	/* radiance spectre visible W/m2/sr */ 
	if(type_out==2)
		integr *= 2*PI/(nbfac*De);		/* luminance cd/m2 ou lm/m2/sr */
	if(type_out==3)
		integr *= 2*PI/(nbfac*Dv);	/* radiance solaire spectre total W/m2/sr */
	  
	integ=integr;
	luminance (0.0, 0.0, h,az, &lumin, &integr);
	lzenit=lumin/integ;
	//printf("integ lumin integr %f %f %f\n",integ,lumin,integr);
	//printf("lzenit %f\n",lzenit);
 
	min = max = lum[0]/integ; 
	for (i = 0; i < nbfac; i++) 
		{
		*(lum+i) = *(lum+i)/integ;        
		if (*(lum+i) < min)
			min = *(lum+i);
		if (*(lum+i) > max)
			max = *(lum+i);
		}


/* IMPRESSION DES CALCULS SOLAIRES */
/*_________________________________*/
 affiche_soleil(latit,longit,jour,mois,jd,h_deb,m_deb, h, sd, st, epsilon, delta, lzenit,De,Se,Dv,Sv);


// ECRITURE DU FICHIER .val à l'heure considérée 
//__________________________
  
		met_extension_heure(temps,extension);
		sprintf(buf,"%s%s",argv[11],extension);
        compose_nom_complet(sortie,s_dir,buf,"val");

        //sprintf(sortie,"%s%d.val",argv[11],kk);
        printf("   %s\n",sortie);
	if ((fp2 = fopen(sortie,"w")) == NULL)
	{
		printf("\n  impossible de creer %s !!\n\n", sortie);
		exit(0);
	}	

	fprintf (fp2,"%d %d %8.5f %8.5f\n", nbfac, nbfac, min, max);   
	for (i = 0; i < nbfac; i++) 
	{
		fprintf (fp2,"f%d 1\n", i+1); 
		
		/* CIEL UNIFORME SI EPSILONE=1 ET DELTA=0.01 */
		if(epsilon == 1 && delta == 0.01)
		     fprintf (fp2,"3180\n");
		else
		{
			//printf("lum enr %8.5f\n", *(lum+i));
			if(*(lum+i)==0)*(lum+i)=0;
		   fprintf (fp2,"   %8.5f\n", *(lum+i));  
		}
	}
  fclose(fp2);
  temps+=pas;

 }

desalloue_double(per_);
desalloue_double(lum);
desalloue_face(face1,nbfac);
creer_OK_Solene();
printf("\n\nFin du Traitement luminance_ciel_temps\n");
printf("_______________________________________________\n");

exit(0);
}


/*__________________________*/
/*			                */
/*     CALCULS SOLAIRES     */
/*__________________________*/

/* numero du jour dans l'annee */
int jdate (mois, jour)  
int mois, jour;        
{
	static short jm[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	return (jm[mois - 1] + jour);
}

/*__________________________________________________________*/
/* Declinaison solaire */ 
double declin (jd) 
int jd;                  
{
 double PI;
 	PI=4*atan(1.);
	return (0.4093 * sin ((2*PI/368) * (jd - 81)));  
}

/*__________________________________________________________*/
/* Altitude solaire */ 
double alt (sd, st,latit)          
double sd,st,latit;
{
 double PI;

 	PI=4*atan(1.);
	return (asin (sin (latit*PI/180.) * sin (sd) - 
		cos (latit*PI/180.) * cos (sd) * cos (st*PI/12.)));  
}

/*__________________________________________________________*/
/* Azimuth solaire */   
double azim (double sd,		//  en radian
			 double st,		// en radian
			 double latit)  // latitude en degre       
{                          
 double PI;
 	 PI=4*atan(1.);
     return (-atan2 (cos(sd) * sin (st*PI/12.),
		-cos (latit*PI/180.) * sin (sd) -
		sin (latit*PI/180.) * cos (sd) * cos (st*PI/12.)));            
}


/*__________________________________________________________*/
/* excentricite */
double exentr (jd) 
int jd; 
{ 
 double PI;
 	 PI=4*atan(1.);
	 return (1.00011 + 0.034221*cos(2*PI*(jd-1)/365) + 0.00128*sin(2*PI*(jd-1)/365) +
			0.000719*cos(2*2*PI*(jd-1)/365) + 0.000077*sin(2*2*PI*(jd-1)/365));
	}

/*__________________________________________________________*/
/* masse d'air optique */
double airmass (double h)
{
 double PI;
 double x,y,z;
 	PI=4*atan(1.);

//	printf("pow %f\n",pow(h*180./PI+3.885,-1.253));
//  printf("h %f\n",h);

	y = pow(h*180./PI+3.885,-1.253);
    z= sin(h);

//printf(" yz %f %f \n",y,z);
//	x = 1. / (sin(h) + 0.15*pow(h*180./PI+3.885,-1.253));
	x = 1/(z + 0.15 * y);
//printf("h = %f air mass %f\n",h,x);

	return (x);
}


/*__________________________________________________________*/
/* calcul de la luminance */  
void luminance (xg, yg, h, az, lumin, integr)
double xg, yg, h, az, *lumin, *integr;
{
	double Z, xi, d, alpha, gamma;
	double fprim, gprim;
	double PI;

 	PI=4*atan(1.);

	Z = PI/2-h;
	d = xg * xg + yg * yg; 
    	if(xg==0)
		{
		if(yg>=0) alpha=PI/2;
		else alpha=3*PI/2;
		}
	else alpha=atan2(yg, xg); 		 /* PI + atan2(-xg, yg); */
/*	if(alpha<0) alpha+=2*PI;*/

//printf("\nxg%6.2f    yg%6.2f",xg, yg);

	xi = asin (sqrt(d) / rayon);
	if (xg == 0 && yg == 0)
		xi = 0.0;
					//printf("\nalph%6.2f    xi%6.2f",alpha*180/PI, xi*180/PI);
	gamma = acos(sin(xi) * sin(Z) * cos(alpha-az) + cos(xi) * cos(Z));
					//printf("\ngam %6.2f\n",gamma*180/PI);

	/* ciel Perez */	
    fprim = 1. + a[0] * exp(a[1]/cos(xi));                   
	gprim = 1. + a[2] * exp(a[3]*gamma) + a[4] * cos(gamma)*cos(gamma);
				//printf("fprim gprim  %f %f \n",fprim,gprim);

	*lumin = fprim * gprim;
	*integr += *lumin * cos(xi);  

}


/*__________________________________________________________*/
/* Lecture des coefficients du modele PEREZ */
void lect_coef_per(filename, per_)
char *filename;
double *per_;
{
 double PI;
 FILE *fper_;
 int i, j, k, imax, jmax;

	
    PI=4*atan(1.);
	imax = 8, jmax = 20;
	fper_ = fopen(filename, "r");  
	if (fper_ == NULL)
		{
		printf("impossible lire %s\n\n", filename);
		exit(0);
		}
   
	for (i = 0; i < imax; i++)
	{
	  for (j = 0; j < jmax; j++)    
		{
		 k= i*jmax + j;
		 fscanf (fper_, "%lf", per_+k); 
		 //printf("per_ %d %f\n",k,*(per_+k));
		}
	}
	fclose(fper_);
}


/*_____________________________________________________________________*/
/* Efficacite lumineuse du modele PEREZ : cas horizontal global diffus */
double effic_lum_diffus(delta,Z,w,interval)
int interval;
double delta, Z, w;
{
 double PI;
 double valeur, a[8], b[8], c[8], d[8];

 PI=4*atan(1.);

/* Coefficents du modele */
	a[0] = 97.24, a[1] = 107.22, a[2] = 104.97, a[3] = 102.39;
	a[4] = 100.71, a[5] = 106.42, a[6] = 141.88, a[7] = 152.23;

	b[0] = -0.46, b[1] = 1.15, b[2] = 2.96, b[3] = 5.59;
	b[4] = 5.94, b[5] = 3.83, b[6] = 1.90, b[7] = 0.35;

	c[0] = 12.00, c[1] = 0.59, c[2] = -5.53, c[3] = -13.95;
	c[4] = -22.75, c[5] = -36.15, c[6] = -53.24, c[7] = -45.27;

	d[0] = -8.91, d[1] = -3.95, d[2] = -8.77, d[3] = -13.90;
	d[4] = -23.74, d[5] = -28.83, d[6] = -14.03, d[7] = -7.98;

	valeur = a[interval] + b[interval]*w + c[interval]*cos(Z) + d[interval]*log(delta);

/*	printf("\n%d glob_h_diffuse_PEREZ %6.3f\n", interval, valeur); */
	return(valeur);
	}


/*__________________________________________________________*/
/* Efficacite lumineuse du modele PEREZ : cas normal direct */
double effic_lum_normal(delta,Z,w,interval)
int interval;
double delta, Z, w;
	{
	double valeur, a[8], b[8], c[8], d[8];

/* Coefficents du modele */
	a[0] = 57.20, a[1] = 98.99, a[2] = 109.83, a[3] = 110.34;
	a[4] = 106.36, a[5] = 107.19, a[6] = 105.75, a[7] = 101.18;

	b[0] = -4.55, b[1] = -3.46, b[2] = -4.90, b[3] = -5.84;
	b[4] = -3.97, b[5] = -1.25, b[6] = 0.77, b[7] = 1.58;

	c[0] = -2.98, c[1] = -1.21, c[2] = -1.71, c[3] = -1.99;
	c[4] = -1.75, c[5] = -1.51, c[6] = -1.26, c[7] = -1.10;

	d[0] = 117.12, d[1] = 12.38, d[2] = -8.81, d[3] = -4.56;
	d[4] = -6.16, d[5] = -26.73, d[6] = -34.44, d[7] = -8.29;

	valeur = a[interval] + b[interval]*w + c[interval]*exp(5.73*Z-5) + d[interval]*delta;
	if(valeur<0) valeur=0;

/*	printf("\n%d glob_h_diffuse_PEREZ %6.3f\n", interval, valeur); */
	return(valeur);
	}


/*__________________________________________________________*/
/*__________________________________________________________*/
/* Ecriture des calculs solaires */

void affiche_soleil(latit,longit,jour,mois,jd,h_deb,m_deb, h ,sd, st, epsilon, delta, lzenit,De,Se,Dv,Sv)
double latit,longit;
int    jour,mois,jd,h_deb,m_deb;
double epsilon, delta;
double h,sd,st;
double lzenit,De,Se,Dv,Sv;
{
 double PI;
    PI=4*atan(1.);

	printf("    Latitude      %+4.2f  \n", latit);       
	printf("    Longitude     %+4.2f  \n", longit);             
    printf("    Date (j/m)    %d/%d   \n\n", jour, mois); 
/*        
	printf(" ** Heure  solaire **     %2.0f h %2.0f mn\n\n", floor(heure), (heure - floor(heure))*100.);  
*/  
       	printf(" ** Heure  solaire **    %d:%d   \n\n", h_deb, m_deb); 

	printf("    Hauteur solaire       %+4.2f\n", h*180./PI); 
	printf("    Declinaison solaire   %+4.2f\n", sd*180./PI);   
	printf("    Azimut solaire        %+4.2f\n\n", azim (sd, st,latit)*180./PI);  
	printf("    Direction du soleil (%+5.5f  %+5.5f  %+5.5f)\n\n", -sin(azim (sd, st,latit))*cos(h),-cos(azim (sd, st,latit))*cos(h), sin(h)); 
	printf("    Correction d'excentricite   %6.3f\n", exentr(jd));       
	printf("    Masse d'air optique         %6.3f\n\n", airmass(h));
   
	
	printf("    coeff. du modele Perez : epsilon %3.2f  delta %3.2f\n", epsilon, delta);  
/*	printf("\n    integr_Perez  %3.5f\n\n", integr); */

	if(type_out==1)
	printf("\n    Radiance au zenith (spectre visible) %6.2f W/m2/sr\n", lzenit);
	else if(type_out==2)
	printf("\n    Radiance au zenith (spectre total) %6.2f W/m2/sr\n", lzenit);
	else if(type_out==3)
	printf("\n    Luminance au zenith  %6.2f cd/m2\n", lzenit);



	printf("\n    irradiance diffuse  De = %8.2f W/m2", De);
	printf("\n    irradiance directe  Se = %8.2f W/m2", Se); 
	printf("\n\n    illuminance diffuse Dv = %8.2f lux", Dv);
	printf("\n    illuminance directe Sv = %8.2f lux", Sv); 
	printf("\n\n");
}

/*__________________________________________________________*/
/* Format de la fonction luminance_ciel_temps */
void usage_ciel()
{
 printf("\n luminance_ciel_temps  \n");

 printf("\n la fonction a comme parametre ENTREE :\n");
 printf("\t ciel_in(.cir)\n"); 
 printf("\t latitude\n");
 printf("\t longitude\n");

 printf("\t jour/mois\n");
 printf("\t hh1:mn1 en TSV\n");
 printf("\t hh2:mn2 en TSV\n");
 printf("\t pas(hh:mn)\n");

 printf("\t epsilon du ciel Perez\n");
 printf("\t delta   du ciel Perez\n");

	printf("\n\t* type_sortie *\n");
	printf("\t\t1 radiance spectre visible W/m2/sr\n");
	printf("\t\t2 radiance solaire spectre total W/m2/sr\n");
	printf("\t\t3 luminance lm/m2/sr ou cd/m2\n\n");
 
 printf("           comme parametres en SORTIE :\n");
 printf("\t    nom du descripteur : luminance\n\n"); 
 printf("NOTA: les fichiers résultats de luminance (.val) sont indexés par l'heure (luminance_--H--.val)n\n"); 
exit(0);                                         
}



