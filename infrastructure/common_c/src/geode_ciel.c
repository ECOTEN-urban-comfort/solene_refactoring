/* geode_ciel.c : DISCRETISATION GEODESIQUE DE LA VOUTE CELESTE */
/*                                                	   */
/*_________________________________________________________*/

// 22 février 2002 D. Groleau F. Miguet 
//		ciel recursif jusqu'à 6 et rotation du ciel pour éviter régularité
// 3 mars 2000 F. Miguet
//		modifie pour faire un ciell à 4 faces
//		avec discretisation t=0
/*
cc geode_ciel.c geomutile.o solutile.o -o geode_ciel lib_solene_94.o -lm
*/

#include<solene.h>


FILE *fp;  

int usage_discret_ciel();
int ecrit_en_tete();
void ecrit0();

int i, recurs, nbf = 4;
double nor[3], rayon, x[3], y[3], z[3];

struct vertex { double x, y, z; };
struct triangle { struct vertex a, b, c; };

/* milieu d'un segment (etendu au rayon de l'hemisphere) */
struct vertex average(v1, v2)
struct vertex *v1;
struct vertex *v2;
	{
	struct vertex m;
	double hypot, ratio;

	m.x = (v1->x + v2->x) / 2;
	m.y = (v1->y + v2->y) / 2;
	m.z = (v1->z + v2->z) / 2;

	hypot = sqrt(m.x*m.x + m.y*m.y + m.z*m.z);
	ratio = rayon / hypot;

	m.x *= ratio;
	m.y *= ratio;
	m.z *= ratio;

	return m;
	}

/* division du triangle parent en 4 triangles plus petits */
void divide_triangle(t, depth)
struct triangle *t;
int depth;
	{
	int subt; 
	struct triangle new_t[4];

	new_t[0].a = t->a;
	new_t[1].b = t->b;
	new_t[2].c = t->c;

	new_t[0].c = new_t[2].a = new_t[3].b = average(&t->a, &t->c);
	new_t[0].b = new_t[1].a = new_t[3].c = average(&t->a, &t->b);
	new_t[1].c = new_t[2].b = new_t[3].a = average(&t->c, &t->b);

	/* recurse if depth not yet reached */
	if (--depth != 0)
		for(subt = 0; subt < 4; subt++)
			divide_triangle(&new_t[subt], depth);
	else          /* dump triangle data */  
		{
		for(subt = 0; subt < 4; subt++)
			{
		x[0]=new_t[subt].b.x, x[1]=new_t[subt].a.x, x[2]=new_t[subt].c.x;
		y[0]=new_t[subt].b.y, y[1]=new_t[subt].a.y, y[2]=new_t[subt].c.y;
		z[0]=new_t[subt].b.z, z[1]=new_t[subt].a.z, z[2]=new_t[subt].c.z;

		normale_avec_3pts (x, y, z, nor);
		fprintf(fp,"f%d 1\n", 4*i+1);
		fprintf (fp,"%10.5f %10.5f %10.5f\n", nor[0], nor[1], nor[2]);  
		fprintf(fp," c0\n     4\n");  
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], y[1], z[1]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[0], y[0], z[0]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[2], y[2], z[2]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], y[1], z[1]);
		
		fprintf(fp,"f%d 1\n", 4*i+2);  
		fprintf (fp,"%10.5f %10.5f %10.5f\n", -nor[0], nor[1], nor[2]);  
		fprintf(fp," c0\n     4\n");  
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], y[1], z[1]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[0], y[0], z[0]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[2], y[2], z[2]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], y[1], z[1]); 
					
		fprintf(fp,"f%d 1\n", 4*i+3);  
		fprintf (fp,"%10.5f %10.5f %10.5f\n", nor[0], -nor[1], nor[2]);
		fprintf(fp," c0\n     4\n");  
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], -y[1], z[1]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[0], -y[0], z[0]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[2], -y[2], z[2]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], -y[1], z[1]); 
			
		fprintf(fp,"f%d 1\n", 4*i+4);  
		fprintf (fp,"%10.5f %10.5f %10.5f\n", -nor[0], -nor[1], nor[2]);
		fprintf(fp," c0\n     4\n");  
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], -y[1], z[1]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[0], -y[0], z[0]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[2], -y[2], z[2]); 
		fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], -y[1], z[1]); 
		i++;
			}
		}
	}


/*____________________________________________*/

main (argc, argv)
int argc;
char *argv[];
	{
	int j;
	char buf[256], *s_dir;   
	struct triangle t;
	double englob[10];
	rayon=0;

	if (argc<3||argc>4)  
		usage_discret_ciel(); 
  
	s_dir = (char *)getenv("PWD");

//	compose_nom_complet(buf, s_dir, argv[1],"cir");	 /* fichier de sortie */
	// la geode est constituée dans un fichier tempeoraire: XTEMP_GEODE.cir

	sscanf (argv[2],"%d", &recurs);  
//	if (recurs < 0 || recurs > 5)  
	if (recurs < 0 || recurs > 6)   
		usage_discret_ciel(); 

	if(argc==4)
		sscanf (argv[3],"%lf", &rayon);
	if (rayon==0) rayon=1;

	//SII Modif DFA 18-04-2006
	fp = createFileTmpSolene("XTEMP_GEODE.cir","w"); 
	if( (fp=createFileTmpSolene("XTEMP_GEODE.cir","w")) == NULL ) {
		printf("\n impossible ouvrir %s/XTEMP_GEODE.cir\n", getenv(SOLENETEMP));
		exit(0);
	}
	//SII

	/* initialisation du triangle parent */
	t.a.x = t.b.x = 0;
	t.b.y = t.c.y = 0;
	t.a.z = t.c.z = 0;
	t.c.x = rayon;
	t.a.y = rayon;
	t.b.z = rayon;

	for (j = 0; j < recurs; j++)
		nbf *= 4;

	printf("\nhemisphere de rayon %.2lf discretise en %d facettes triangulaires\n\n", rayon, nbf);

/* ECRITURE EN-TETE */
	englob[0] = 0.;
	englob[1] = englob[4] = englob[6] = englob[7] = englob[9] = rayon;
	englob[2] = englob[3] = englob[5] = englob[8] = -rayon;    
	
   	ecrit_en_tete(fp, nbf, nbf, englob);    

	if(recurs == 0)
		ecrit0();
	else
		divide_triangle(&t, recurs);
	
	// close le fichier temporaire
	fclose(fp);
	// applique rotation au ciel
	// *** modif laurent malys 22-01-2010 ***
#ifdef LINUX
    	sprintf(buf,"rot Z 7 %s\XTEMP_GEODE %s",getenv(SOLENETEMP),argv[1]);
#else
    	sprintf(buf,"rot Z 7 %s//XTEMP_GEODE %s",getenv(SOLENETEMP),argv[1]);
#endif 
	printf("%s\n",buf);
	system(buf);
	// delete fichier temporaire
	cleanFileTmpSolene("XTEMP_GEODE.cir");

    creer_OK_Solene();
	exit (0);
	}


/*__________________________________________________________*/
/* Ecrit0 */
void ecrit0()
	{
	x[1]=x[0]=0.0;
	x[2]=1.0;
	y[0]=y[2]=0.0;
	y[1]=1.0;
	z[1]=z[2]=0.0;
	z[0]=1.0;

	normale_avec_3pts (x, y, z, nor);
	fprintf(fp,"f1 1\n");
	fprintf (fp,"%10.5f %10.5f %10.5f\n", nor[0], nor[1], nor[2]);  
	fprintf(fp," c0\n     4\n");  
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], y[1], z[1]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[0], y[0], z[0]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[2], y[2], z[2]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], y[1], z[1]);
		
	fprintf(fp,"f2 1\n");  
	fprintf (fp,"%10.5f %10.5f %10.5f\n", -nor[0], nor[1], nor[2]);  
	fprintf(fp," c0\n     4\n");  
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], y[1], z[1]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[0], y[0], z[0]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[2], y[2], z[2]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], y[1], z[1]); 
					
	fprintf(fp,"f3 1\n");  
	fprintf (fp,"%10.5f %10.5f %10.5f\n", nor[0], -nor[1], nor[2]);
	fprintf(fp," c0\n     4\n");  
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], -y[1], z[1]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[0], -y[0], z[0]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[2], -y[2], z[2]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",x[1], -y[1], z[1]); 
			
	fprintf(fp,"f4 1\n");  
	fprintf (fp,"%10.5f %10.5f %10.5f\n", -nor[0], -nor[1], nor[2]);
	fprintf(fp," c0\n     4\n");  
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], -y[1], z[1]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[0], -y[0], z[0]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[2], -y[2], z[2]); 
	fprintf(fp,"%10.5f %10.5f %10.5f\n",-x[1], -y[1], z[1]); 
	}



/*__________________________________________________________*/
/* Format de la fonction geode */
int usage_discret_ciel()
	{
	printf("\n   *geode_ciel*  fichier_out(.cir)  precision  rayon(1 par default)\n");
	printf("\tprecision\t0  :   4 faces\n");
	printf("\t\t\t0  :    4 faces\n");
	printf("\t\t\t1  :   16 faces\n");
	printf("\t\t\t2  :   64 faces\n");
	printf("\t\t\t3  :  256 faces\n");
	printf("\t\t\t4  : 1024 faces\n");
	printf("\t\t\t5  : 4096 faces\n\n");
	printf("\t\t\t6  :16384 faces\n\n");
	exit(0);                                         
	}











