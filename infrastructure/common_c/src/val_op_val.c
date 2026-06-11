/* val_op_val.c       operations entre fichier de valeurs .val */

// D.GROLEAU modif fev 2006 (enlève restriction sur min = max), voir MODIF fev 2006
/*

cc val_op_val.c  -o val_op_val geomutile.o solutile.o lib_solene_94.o -lm

*/

#include <solene.h>

// function
int quel_seuil();
void format_entree_val_op_val();

/*_________________________________________________________________*/
main(argc,argv)
int argc;char **argv;
{
 FILE *fval1,*fval2,*fval3;
 char nom1[64],nom2[64],nom3[64],c,c1,*s_dir;
 int op,nbfac,numax,nbcontour,i,oui,j,nofac;
 float val1,val2,val3,vmin,vmax,nombre;
 int trans_val,nbseuil1,nbseuil2,is1,is2;
 float vmin1,vmax1,vmin2,vmax2;

   if(argc<5) format_entree_val_op_val();
	vmin=1000000.;vmax=-vmin;

   s_dir=(char *)getenv("PWD");

   printf("Fonction Solene : val_op_val\n");

    /* lecture parametres commande */

   /* lit le fichier des valeurs */
 compose_nom_complet(nom1,s_dir,argv[1],"val");


        printf("\nOperation : \n %s\n",nom1);
        if((fval1=fopen(nom1,"r"))==NULL)
            { printf("\nimpossible ouvrir %s\n",nom1);
	      exit(0);
	    }
	sscanf(argv[2],"%c",&c);
	printf(" %s\n",argv[2]);
/*	printf(" %c\n",c);*/
	if(c=='+') op=1;
	else if(c=='-') op=2;
	else if(c=='x') op=3;
	else if(c=='/') op=4;
	else if(c=='L')
		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='T') op=5;         /* LT  < */
		  else if(c1=='E') op=7;    /* LE <= */
		}
	else if(c=='G') 
		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='T') op=6;         /* GT  > */
		  else if(c1=='E') op=8;    /* GE <= */
		}
	else if(c=='E')
		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='Q') op=9;          /* EQ = */
		  else if(c1=='T') op=11;    /* ET   */
		}
	else if(c=='N')
		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='E') op=10;    /* != */
		}
	else if(c=='O') 
 		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='U') op=12;    /* OU */
		}
	else if(c=='C') 
 		{ sscanf(argv[2],"%c%c",&c,&c1);
		  if(c1=='R') op=13;    /* OU */
		}
	else format_entree_val_op_val();
	sscanf(argv[3],"%c",&c);
	if(c>='0'&& c<'9') 
		{ sscanf(argv[3],"%f",&nombre);
		  printf(" %f",nombre);
		  oui=0;
		}
	else
		{ oui=1;
		 compose_nom_complet(nom2,s_dir,argv[3],"val");
    		  printf(" %s ",nom2);
      	 	  if((fval2=fopen(nom2,"r"))==NULL)
         	   { printf("\nimpossible ouvrir %s\n",nom2);
	  	     exit(0);
	  	   }
		}
	compose_nom_complet(nom3,s_dir,argv[4],"val");
        printf("\n\nResultat dans %s\n\n",nom3);
        if((fval3=fopen(nom3,"w"))==NULL)
            { printf("\nimpossible ouvrir %s\n",nom3);
	      exit(0);
	    }

  /* si op  entraine reponse Binaire ; alors */
  /* on reprend la valeur du 1er fichier */
  trans_val=0; 
   if(op!=13 && op>=5)
      {  
        if(argc==6) { sscanf(argv[5],"%c",&c);
		      if(c=='T') trans_val=1;
                      else format_entree_val_op_val();
                     }
      }
   else if(op==13) /* operation croisement de valeurs TRI */
     { if(argc<7 || oui==0)format_entree_val_op_val();
      sscanf(argv[5],"%d",&nbseuil1);
      sscanf(argv[6],"%d",&nbseuil2);
	  printf("croisement avec %d seuils par %d seuils\n",nbseuil1,nbseuil2);
     }
/*  on traite */

	fscanf(fval1,"%d %d %f %f",&nbfac,&numax,&vmin1,&vmax1);
	printf("min1 %f max1 %f\n",vmin1,vmax1);
// MODIF fev 2006
// if(vmin1==vmax1) {printf("min = max\n"); exit(0);}
// Fin MODIF fev 2006

	if(oui)
	{fscanf(fval2,"%d %d %f %f",&i,&numax,&vmin2,&vmax2);
	 printf("min2 %f max2 %f\n",vmin2,vmax2);
// MODIF fev 2006
//	 if(vmin2==vmax2) {printf("min = max\n"); exit(0);}
// Fin MODIF fev 2006
	}
	if(oui && nbfac!=i) 
		{ printf(" fichiers f1 et f2 incompatibles \n");
		  exit(0);
		}
	fprintf(fval3,"%d %d %15.5f %15.5f\n",nbfac,numax,vmin,vmax);
	for(i=0;i<nbfac;i++)
	 { fscanf(fval1,"\n%c %d %d ",&c,&nofac,&nbcontour);
	   if(oui)fscanf(fval2,"\n%c %d %d",&c,&nofac,&nbcontour);
	   fprintf(fval3,"f%d %d\n",nofac,nbcontour);
           for(j=0;j<nbcontour;j++)
	     { fscanf(fval1,"%f",&val1);
               if(oui)fscanf(fval2,"%f",&val2); else val2=nombre;
			if(op==1) val3=val1+val2;
			else if(op==2) val3=val1-val2;
			else if(op==3) val3=val1*val2;
			else if(op==4) 
				{ if(val2)val3=val1/val2;
				  else val3=0;
				}
			else if(op==5) 
				{ if(val1 < val2)val3=1;
				  else val3=0;
				}
			else if(op==6) 
				{ if(val1 > val2)val3=1;
				  else val3=0;
				}
			else if(op==7) 
				{ if(val1 <= val2)val3=1;
				  else val3=0;
				}
			else if(op==8) 
				{ if(val1 >= val2)val3=1;
				  else val3=0;
				}
			else if(op==9) 
				{ if(val1 == val2)val3=1;
				  else val3=0;
				}
			else if(op==10) 
				{ if(val1 != val2)val3=1;
				  else val3=0;
				}
			else if(op==11) 
				{ if(val1 && val2)val3=1;
				  else val3=0;
				}
			else if(op==12) 
				{ if(val1 || val2)val3=1;
				  else val3=0;
				}
			else if(op==13) 
				{ is1=quel_seuil(vmax1,vmin1,nbseuil1,val1);
                  is2=quel_seuil(vmax2,vmin2,nbseuil2,val2);
				  printf("\nseuil1 %d seuil2 %d\n",is1,is2);
				  val3=(is1-1)*nbseuil2+is2;
				}

                   if(trans_val)
                         { if(val3) val3=val1;
                         }

		        if(val3 < vmin) vmin=val3;
		        if(val3 > vmax) vmax=val3;
		        fprintf(fval3,"%f\n",val3);
		}
	   }
	rewind(fval3);
	fprintf(fval3,"%d %d %15.5f %15.5f\n",nbfac,numax,vmin,vmax);

    printf("Fin du traitement : val_op_val\n");
    creer_OK_Solene();

}
/*_________________________________________________________________*/
int quel_seuil(vmax,vmin,nbseuil,val)
float vmax,vmin,val;
int nbseuil;
{ int is;  /* renvoie seuil is entre 1 et nbseuil */
  is=1+(val-vmin)*(nbseuil)/(vmax-vmin);
  if(is > nbseuil) is=nbseuil;
  return(is);
}
/*_________________________________________________________________*/
void format_entree_val_op_val()
{
  printf("\n *val_op_val* f1_in(.val) OP f2_in(.val)  f3_out(.val) [T]\n\n");
  printf("NOTA: OP vaut :  + - x / GT LT GE LE EQ NE ET OU CR\n\n");
  printf("NOTA: le parametre T transmet la valeur du 1er fichier\n");
  printf("         si OP  GT LT GE LE EQ NE ET OU\n\n");
  printf("         et si resultat OUI , au lieu de mettre val=1\n\n");
  printf("NOTA: format de Operation CR \n\n");
  printf("val_op_val f1(.val) CR f2(.val)  f3(.val) nbseuil1 nbseuil2\n\n");
 exit(0);
}
