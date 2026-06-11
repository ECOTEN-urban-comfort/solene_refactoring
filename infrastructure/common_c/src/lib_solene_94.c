/*
cc -c lib_solene_94.c -lm
*/

/* #include</home/cerma.0/marenne/newsolene/solene_94.h>  */

#include<solene.h>

/*________________________________________________________________________*/
int creer_OK_Solene()
{
 FILE *fp;
 
// Modif SII FR060317
  char pcTemp[5000];
#ifdef LINUX
  sprintf( pcTemp, "%s/OK_SOLENE", getenv( SOLENETEMP ) );
#else  
  sprintf( pcTemp, "%s\\OK_SOLENE", getenv( SOLENETEMP ) );
#endif
  fp=fopen(pcTemp,"w"); 
// Fin modif SII FR060317

 if(fp==NULL) 
	{ printf("impossible creer OK_SOLENE\n");
	  exit(0);
	}
 fclose(fp);
 return(1);
}




/*________________________________________________________________________*/
/**
 * SII Creation DFA 18-06-2006
 * modification laurent malys 22-01-2010 pour linux
 */
FILE* createFileTmpSolene(char* tmpFileName, char* mode) {
	FILE *fp = NULL;
	char pcTemp[5000];
#ifdef LINUX
	sprintf( pcTemp, "%s/%s", getenv( SOLENETEMP ), tmpFileName );
#else
	sprintf( pcTemp, "%s\\%s", getenv( SOLENETEMP ), tmpFileName );	
#endif	
	fp=fopen(pcTemp,mode); 
	return fp; // peut être null, donc tester le retour
}


/*________________________________________________________________________*/
/**
 * SII Creation DFA 18-06-2006
 * modification laurent malys 22-01-2010 pour linux
 * 'rm' remplace 'del'
 */
void cleanOkSolene() {
	char buf[5000];
		
	if (existOkSolene()) {
#ifdef LINUX
		sprintf(buf,"rm %s/OK_SOLENE",getenv(SOLENETEMP));
#else
		sprintf(buf,"del %s\\OK_SOLENE",getenv(SOLENETEMP));
#endif
		system(buf);
	}
}


/*________________________________________________________________________*/
/**
 * SII Creation DFA 18-06-2006
 * modification laurent malys 22-01-2010 pour linux
 * 'rm' remplace 'del'
 */
void cleanFileTmpSolene(char *filename) {
	char buf[5000];
#ifdef LINUX
	sprintf(buf,"rm %s/%s",getenv(SOLENETEMP),filename);
#else
	sprintf(buf,"del %s\\%s",getenv(SOLENETEMP),filename);
#endif 

	system(buf);
}


/*________________________________________________________________________*/
int compose_nom_complet(buf,s_dir,argument,ext)
char *buf,*s_dir,*argument,*ext;
{ 
  /* si le nom "argument commence par / ; on considere nom complet */
  /* sinon on ajoute dir "s_dir" */
  /* on rajoute tjs l'extension "ext" */

  sprintf(buf,"%s.%s",argument,ext);
/*
  if(buf[0]=='/') sprintf(buf,"%s.%s",argument,ext);
  else  sprintf(buf,"%s/%s.%s",s_dir,argument,ext);
*/
    return(1);

}

/*________________________________________________________________________*/
int compose_nom_complet_sans_ext(buf,s_dir,argument)
char *buf,*s_dir,*argument;
{ 
  /* si le nom "argument commence par / ; on considere nom complet */
  /* sinon on ajoute dir "s_dir" */
  /* on rajoute tjs l'extension "ext" */

  sprintf(buf,"%s",argument);

  /*
  if(buf[0]=='/') sprintf(buf,"%s",argument);
  else  sprintf(buf,"%s/%s",s_dir,argument);
  */
  return(1);
}

/*________________________________________________________________________*/
ajuste_face(nbf,fac)
int nbf;
struct modelisation_face *fac;
{struct contour *pcont;
 struct circuit *pcir;
 int i;
      
     for(i=0;i<nbf;i++)
        {pcont=(fac+i)->debut_projete;
			while(pcont)
				{pcir=pcont->debut_support;
				 fenetre_circuit(pcir);
				 sensz(pcir);
				 fentra(pcir);
				 pcir=pcont->debut_interieur;
				 while(pcir)
					{sensz(pcir);
					 pcir=pcir->suc;
					}
				 pcont=pcont->suc;
				}
       }
	 return(1);
}


/*________________________________________________________________________*/
/*
 * test si fichier nom existe 
 */
int
test_si_nom_existe(nom)
char	*nom;
{ 
	// return(1) si existe
 char  	buf[64];
 FILE *fp;

	sprintf(buf,"%s", nom);
//	printf("%s\n",buf);
	if((fp = fopen(buf,"r")) == NULL)
	{
		return(0);
	}
	else
	{
		fclose(fp);
		return(1); 
	}
}

//_____________________________________________________
void met_extension_heure (temps,extension)
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

/*________________________________________________________________________*/
/**
 * SII Creation DFA 18-06-2006
 */
int existOkSolene() {

	char buf[5000];
	sprintf(buf,"%s/OK_SOLENE",getenv(SOLENETEMP));
	return test_si_nom_existe(buf);
}
