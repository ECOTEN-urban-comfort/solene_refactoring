/*

cc rot.c  solutile.o geomutile.o lib_solene_94.o -o rotz -lm

*/
/* rot.c  ROTATION EN PLAN aoutour de oz d'un fichier de faces */
/* commande rot XouYouZ angle f_in f_out   angle>0 sens trigo */


#include<solene.h>


struct modelisation_face *alloue_face();

 struct modelisation_face *ff;
 int nbff;
 char  axe;

FILE *fp;
/*_________________________________________________________________*/
main(argc,argv)
 int argc;char **argv;
{char 	buf[512],*s_dir;
 double angle,x[3],pi,englob[10];
 int j,nb;
 
 if(argc<4)format_entree();

	s_dir=(char *)getenv("PWD");

     init_verif_pteur_solene();
     obs.xo=0; obs.yo=0; obs.zo=0;
     nb_etat=0;
     pi=4*atan(1.); 

  sscanf(argv[1],"%c",&axe);
  printf("\n axe de rotation = %c ",axe);

  sscanf(argv[2],"%lf",&angle);
  printf("\n angle de rotation = %lf ",angle);
  angle=angle*pi/180.; 
  compose_nom_complet(buf,s_dir,argv[3],"cir");
  if((fp=fopen(buf,"r"))==NULL)
        { printf("\n impossible ouvrir %s\n",buf); exit(0);}
   lit_en_tete(fp,&nbff,&nomax,englob);


   /* que devient la boite englobante ? */
   /*
   if(axe == 'z' || axe =='Z')
    {   rot_z_englob(englob,angle);
    }
   else if(axe == 'x' || axe =='X')
    {   rot_x_englob(englob,angle);
    }
   else if(axe == 'y' || axe =='Y')
    {   rot_y_englob(englob,angle);
    }
	*/

   ff=alloue_face(nbff,35);
   lit_fic_cir3d(fp,nbff,ff);
   fclose(fp);
        
  /* calcul de la rotation */ 
     
   for(j=0;j<nbff;j++) 
    {
	  if(axe == 'z' || axe =='Z')
       {
	    rot_z_face(angle,ff+j,1);
       }
	  else if(axe == 'x' || axe =='X')
       {
	    rot_x_face(angle,ff+j,1);
       }
	  else if(axe == 'y' || axe =='Y')
       {
	    rot_y_face(angle,ff+j,1);
       }
    }

/* stocke le fichier */

   compose_nom_complet(buf,s_dir,argv[4],"cir");
   fp=fopen(buf,"w");
    if(fp==NULL) 
	{ printf("impossible creer %s\n",buf);
	  exit(0);
	}
	   ecrit_en_tete(fp,nbff,nomax,englob);
   output_ff(ff,nbff,1,0,fp,&nb,&nomax);
   fclose(fp);
   printf("\n");
  		creer_OK_Solene();

	desalloue_fface(ff,nbff);
}

/*_________________________________________________________________*/
format_entree()
{
  printf("\n    *rotz* axe(X,Y ou Z) angle(degre >0 sens trigo) fichier_in(.cir) fichier_out(.cir)     \n\n");
   exit(0);
}
/*_________________________________________________________________*/
/*____________________________________________________________________*/
int rot_z_face(angle,face,projete)
struct modelisation_face *face;
int projete;
double angle;
{int i;
 double co,si,tempo;
 struct contour *pcont;
 struct circuit *pcir;

   co=cos(angle);   si=sin(angle);

   tempo=face->vnorm[0];
   face->vnorm[0]=tempo*co-face->vnorm[1]*si;
   face->vnorm[1]=tempo*si+face->vnorm[1]*co;
    
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;

      while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
           {tempo=pcir->x[i];
            pcir->x[i]=tempo*co-pcir->y[i]*si;
            pcir->y[i]=tempo*si+pcir->y[i]*co;
	   }
	 pcir=pcont->debut_interieur;
         while(pcir)
	    {for(i=0;i<pcir->nbp;i++)
	       {tempo=pcir->x[i];
                pcir->x[i]=tempo*co-pcir->y[i]*si;
                pcir->y[i]=tempo*si+pcir->y[i]*co;
	       }
	     pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}

/*____________________________________________________________________*/
int rot_x_face(angle,face,projete)
struct modelisation_face *face;
int projete;
double angle;
{int i;
 double co,si,tempo;
 struct contour *pcont;
 struct circuit *pcir;

   co=cos(angle);   si=sin(angle);

   tempo=face->vnorm[1];
   face->vnorm[1]=tempo*co-face->vnorm[2]*si;
   face->vnorm[2]=tempo*si+face->vnorm[2]*co;
    
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;

      while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
           {tempo=pcir->y[i];
            pcir->y[i]=tempo*co-pcir->z[i]*si;
            pcir->z[i]=tempo*si+pcir->z[i]*co;
	   }
	 pcir=pcont->debut_interieur;
         while(pcir)
	    {for(i=0;i<pcir->nbp;i++)
	       {tempo=pcir->y[i];
                pcir->y[i]=tempo*co-pcir->z[i]*si;
                pcir->z[i]=tempo*si+pcir->z[i]*co;
	       }
	     pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}

/*____________________________________________________________________*/
int rot_y_face(angle,face,projete)
struct modelisation_face *face;
int projete;
double angle;
{int i;
 double co,si,tempo;
 struct contour *pcont;
 struct circuit *pcir;

   co=cos(angle);   si=sin(angle);

   tempo=face->vnorm[0];
   face->vnorm[0]=tempo*co+face->vnorm[2]*si;
   face->vnorm[2]=-tempo*si+face->vnorm[2]*co;
    
   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;

      while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
           {tempo=pcir->x[i];
            pcir->x[i]=tempo*co+pcir->z[i]*si;
            pcir->z[i]=-tempo*si+pcir->z[i]*co;
	   }
	 pcir=pcont->debut_interieur;
         while(pcir)
	    {for(i=0;i<pcir->nbp;i++)
	       {tempo=pcir->x[i];
                pcir->x[i]=tempo*co+pcir->z[i]*si;
                pcir->z[i]=-tempo*si+pcir->z[i]*co;
	       }
	     pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}


/*____________________________________________________________________*/
rot_z_englob(englob,angle)
double *englob;
double angle;
{int i;
 double co,si,tempo;

     co=cos(angle);   si=sin(angle);

     for(i=2;i<10;i+=2)
           {tempo=englob[i];
            englob[i]=tempo*co-englob[i+1]*si;
            englob[i+1]=tempo*si+englob[i+1]*co;
	   }

}


/*-----------------------------------------------------------*/
int output_ff(face,nb,projete,itranf,pf,nbf,nomax)
struct modelisation_face *face;
int nb,projete,itranf,*nbf,*nomax;
FILE *pf;
{
              /* ecrit sur fichier pf,les nb faces; contour projete si projete=1; */
              /*                                    contour dessin  si projete=1; */
              /* en appliquant Tranfo inverse_Normalise  si itranf */
              /* et en mettant a jour nb de faces du fichier, nb contour, nomax */
  struct contour *pcont;
  struct circuit *pcir;
  int i,nbcont,nbtrou;



  for(i=0;i<nb;i++) 
    { 
     if(projete) pcont=face[i].debut_projete; else pcont=face[i].debut_dessin;
     if(pcont)
       {*nbf=*nbf+1; if(face[i].nofac_fichier>*nomax) *nomax=face[i].nofac_fichier;
        nbcont=nb_contour_face(face+i,projete);
        fprintf(pf,"f%d %d\n",face[i].nofac_fichier,nbcont);
        fprintf(pf," %lf  %lf  %lf\n",face[i].vnorm[0],face[i].vnorm[1],face[i].vnorm[2]);
 
        while(pcont)
         { nbtrou=nb_trou_contour(pcont);
           pcir=pcont->debut_support;
                       /* support ; en principe 1 seul */
           fprintf(pf,"c%d\n",nbtrou);
           output_circuit_sur_fichier(pcir,pf,itranf);
           pcir=pcont->debut_interieur;
                      /* les trous */
           while(pcir)
              { fprintf(pf,"t%\n");
                output_circuit_sur_fichier(pcir,pf,itranf);
                pcir=pcir->suc;
              }
           pcont=pcont->suc;
        }
     }
   }
}
