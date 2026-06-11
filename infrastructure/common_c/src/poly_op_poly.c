/*
cc  -c  poly_op_poly.c  -lm
*/
/* reunion de curtis et singul */
#include<solene.h>

int option_sup_surf =1;   /* option SUPPRESSION de SURFACE ds FACE-OP-FACE 1=oui */
int option_calcul_z =1;   /* option CALCUL du Z ds FACE-OP-FACE 1=oui */
                          /* utilise ds singul.c epure_polygone */
			  /* et dans face_op_face.c             */
int methode_non_singul;

struct spol *copy_poly_poly();

extern double coef_discol;
struct liste_point *test_sup_inter1();
struct liste_point *test_sup_inter();
/*____________________________________________________________________*/
/* retour = 0  cir0 et cir1 se coupent                                */
/*        =-1  cir0 et cir1 disjoint				      */
/*        =1   cir1 est inclus dans cir0			      */
/*        =2   cir0 est inclus dans cir1	                      */
/*____________________________________________________________________*/

int masque(pcont,ornotand)
struct contour *pcont;
int ornotand;
{ struct spol *poly1,*poly2;
  int i,ind;
  double sauv;
  struct contour *cont_resul;
  struct circuit *pcir,*pp0,*pp1;

 sauv=coef_discol;

 pp0=alloue_circuit(12);

copie_cir(&cir0,pp0);

 pp1=alloue_circuit(12);

copie_cir(&cir1,pp1);

   debut_inter=NULL;

while(1)
 { cont_resul=alloue_contour(12);
  /* printf("\n\n masque pcont = %ld \n\n",pcont); */
  /*  sensz(&cir1); */
  
    colpt(&cir0,&cir1);

/*
    printf("\n\n circuit situe derriere \n");
    liste_circuit_ecran(&cir0); 
    printf("\n\n circuit situe devant \n");
    liste_circuit_ecran(&cir1);
*/
   lcir0=lcopie(&cir0); lcir1=lcopie(&cir1);

/*
 printf("liste lcir ecran AVANT intcor\n");
 liste_lcir_ecran(lcir0); liste_lcir_ecran(lcir1); 
*/
   debut_inter=NULL;
   /* calcul des intersections */
   intcor();
   z_cir1();

 
   poly1=copie_lcir_spol(lcir0); poly2=copie_lcir_spol(lcir1);



    /* desalloue les inters et les liste_circuits */
   desalloue_inters(); debut_inter=NULL;
   desalloue_liste_point(lcir0,1); desalloue_liste_point(lcir1,2);
   lcir0=NULL;  lcir1=NULL;

    /* test si peut utiliser Methode Non SINGULARITE */
    quelle_methode(poly1);
    if(methode_non_singul)quelle_methode(poly2);
    if(methode_non_singul)
       { non_singularite++; i=0;
       }
    else 
       { singularite++;
         typage_des_aretes_communes(poly1,poly2);
         i=typage_des_aretes_int_ext(poly1,poly2);
         if(i!=999) i=typage_des_aretes_int_ext(poly2,poly1);
       }
  /*  printf("methode_non_singul=%d\n",methode_non_singul);*/

  
if(i!=999)
  {  /* Resultat du typage */
/*
        printf("\nPOLY 1  nb pts = %d\n",poly1->nbp);
    for(i=0;i<poly1->nbp;i++)
        printf("   Statut Pt %2d :  %c  %c  %5.2lf %5.2lf\n",i+1,poly1->statut_point[i],poly1->statut_arete[i],poly1->x[i],poly1->y[i]);
    printf("\nPOLY 2  nb pts = %d\n",poly2->nbp);
    for(i=0;i<poly2->nbp;i++)
        printf("   Statut Pt %2d :  %c  %c  %5.2lf %5.2lf\n",i+1,poly2->statut_point[i],poly2->statut_arete[i],poly2->x[i],poly2->y[i]);
*/    

    for(i=0;i<poly1->nbp-1;i++)  poly1->traite_arete[i]='O';
    for(i=0;i<poly2->nbp-1;i++)  poly2->traite_arete[i]='O';

    if(ornotand==1)ind=poly_union_poly(poly1,poly2,cont_resul);
    else if(ornotand==2)ind=poly_moins_poly(poly1,poly2,cont_resul);
    else if(ornotand==3)ind=poly_inter_poly(poly1,poly2,cont_resul);

    /* desalloue */
    desalloue_poly(poly1); desalloue_poly(poly2);

   /* printf("\n RESULTAT CURTIS \n");  */     
    /*liste_struct_contour_ecran(cont_resul); */
    if(ind!=999)break;
  } 

  coef_discol=coef_discol*10;
  printf("\n PB dans Masque DISCOL = %lf \n",coef_discol*DISCOL); 
  if(coef_discol*DISCOL>1)
      { printf("\n\n circuit situe derriere \n");
        liste_circuit_ecran(pp0); 
        printf("\n\n circuit situe devant \n");
        liste_circuit_ecran(pp1);
         desalloue_contour(cont_resul); 
	 desalloue_circuit(pp0,12);desalloue_circuit(pp1,12);
         coef_discol=sauv;
         return(-1);
      }
  pb_masque++; if(colle_max<coef_discol*DISCOL)colle_max=coef_discol*DISCOL;
  desalloue_contour(cont_resul);
 }


/* verifie sens du circuit */
/*  pcir=cont_resul->debut_support;
  if(surface(pcir->nbp,pcir->x,pcir->y)>0)
	printf("\n resultat dans le mauvais sens\n");*/
                

 if(pcont->debut_support==NULL){pcont->debut_support=cont_resul->debut_support;}
  else {/* rajoute au debut */
	pcir=cont_resul->debut_support;
        while(pcir->suc)pcir=pcir->suc;  pcir->suc=pcont->debut_support; 
        pcont->debut_support=cont_resul->debut_support;
       }
libere_contour(cont_resul,2);
 coef_discol=sauv;
 desalloue_circuit(pp0,12);desalloue_circuit(pp1,12);
 return(ind);
}

/*--------------------------------------------------------------*/
int liste_lcir_ecran(lc)
struct liste_point *lc;
{struct liste_point *lcir;
 lcir=lc;
 printf("\n"); 
 while(lcir)
   {printf("\n  lcir = %ld x = %lf y = %lf  z = %lf",lcir,lcir->x,lcir->y,lcir->z);
    printf(" noinit = %d statut = %c ",lcir->noinit,lcir->statut);
    lcir=lcir->suc;
   }
}

/*--------------------------------------------------------------*/

int intcor()
{int i,j,ii,jj;
 double xint,yint,t1,t2;
      
    for(j=0;j<cir0.nbp-1;j++)
       {
     /* printf("\n\n ****** cherche inter cir0 arete %d *\n",j);*/
      
         /* fenetre arete j de cir0 */
	         fen_arete(&cir0,j,fen);

        for(i=0;i<cir1.nbp-1;i++)
	 { /* verifie si l'intersection est a calculer */ 
               jj=lit_intersecte(j,i);
         
          /*  printf("\n                     et cir1 arete %d",i); */
            if(jj){/*printf("\n inter a ne pas traitee ");*/}
           if(jj==0)
             {/* fenetre arete i de cir1 */
  	         fen_arete(&cir1,i,fen+1);
	      /* test si les fenetres des 2 aretes se coupent */
	  if(recoup(fen,fen+1)!=-1)
	   {/* cherche inter */ 
	      ii=inter(&cir0.x[j],&cir0.y[j],&cir1.x[i],&cir1.y[i],&xint,&yint,&t1,&t2);
	      if(ii==1)
	      {
              /* printf("\n t1 = %lf t2 = %lf xint = %lf yint = %lf",t1,t2,xint,yint);*/

		intersecte(j,i);

               if(t1!=0 && t1!=1 && t2!=0 && t2!=1)
		      {/* c'est une intersection franche */
		        ajout_point(lcir0,'i',j,i,xint,yint,&cir0,&cir1);
			ajout_point(lcir1,'i',i,j,xint,yint,&cir1,&cir0);
               	      }
                else if(t1==0 && t2==0)
		      {verif_intersecte(j-1,i);
		       verif_intersecte(j,i-1);
		       verif_intersecte(j-1,i-1);
		       statut_point(lcir0,'c',j);
		       statut_point(lcir1,'c',i);
		      }
                else if(t1==0 && t2!=0 && t2!=1)
		      {verif_intersecte(j-1,i);
	               ajout_point(lcir1,'c',i,j,cir0.x[j],cir0.y[j],&cir1,&cir0);
		       statut_point(lcir0,'c',j);
		      }
                else if(t2==0 && t1!=0 && t1!=1)
		      {verif_intersecte(j,i-1); 
                       ajout_point(lcir0,'c',j,i,cir1.x[i],cir1.y[i],&cir0,&cir1);
		       statut_point(lcir1,'c',i);
                      }
                else if(t1==0 && t2==1)
		     {verif_intersecte(j-1,i);
		      verif_intersecte(j-1,i+1); 
                      verif_intersecte(j,i+1);
		      intersecte(j,i+1);
		      statut_point(lcir0,'c',j);
		      statut_point(lcir1,'c',i+1);
                     }
                else if(t1==1 && t2==0)
		     {verif_intersecte(j,i-1);
                      intersecte(j+1,i);  intersecte(j+1,i-1);  
                      statut_point(lcir0,'c',j+1);
		      statut_point(lcir1,'c',i);
                     }
		else if(t1==1 && t2==1)
		     {intersecte(j+1,i);  intersecte(j+1,i+1);
                      if(i==cir1.nbp-2)verif_intersecte(j,0);
                      else intersecte(j,i+1);
                      statut_point(lcir0,'c',j+1);
		      statut_point(lcir1,'c',i+1);  
                     }
	        else if(t1==1 && t2!=0 && t2!=1)
		     {if(j==cir0.nbp-2)verif_intersecte(0,i);
                      else intersecte(j+1,i);
                      ajout_point(lcir1,'c',i,j,cir0.x[j+1],cir0.y[j+1],&cir1,&cir0);
		      statut_point(lcir0,'c',j+1);
                     }	 
		else if(t2==1 && t1!=0 && t1!=1)
		     {if(i==cir1.nbp-2)verif_intersecte(j,0);
                      else intersecte(j,i+1); 
                      /* pour pb */ intersecte(j,i);
		      ajout_point(lcir0,'c',j,i,cir1.x[i+1],cir1.y[i+1],&cir0,&cir1);
		      statut_point(lcir1,'c',i+1);
                     }
	     }
           }
          }
	 }
       }
}
/*--------------------------------------------------------------*/
int ajout_point(lcir,car,no1,no2,x,y,pcira,pcirb)
struct liste_point *lcir;
struct circuit *pcira,*pcirb;
char car;
int no1,no2;
double x,y;

{struct liste_point *lc,*lcour,*lcpred,*lcird;
 double dist,xint[2],yint[2];
 int no3,ii;
 double xt[2],yt[2];
 double xxt[2],yyt[2];
 double t1,t2;
 /*printf("\n  AJOUT POINT no1 = %d no2 = %d ",no1,no2);*/
 dist=0.; lcird=lcir;
 while(lcir->noinit!=no1)lcir=lcir->suc;
  lcpred=lcir;
  lc=lcir->suc;
  while(1)
   {if(lc->noinit<0)
	{dist=test_distance(lcir->x,lcir->y,lc->x,lc->y,x,y);}
         if(dist==-999.999)
       /* PB on agrandit la fenetre pour reclasser les 2 intersections */
         {printf("\n INTER CONFONDU\n");
/*printf("\n xint = %lf %lf yint = %lf %lf\n",1000*x,1000*(lc->x),1000*y,1000*(lc->y));
          liste_circuit_ecran(pcira);liste_circuit_ecran(pcirb);*/
         }
         if(dist>0 || lc->noinit>=0)
	   {/* insere l'intersection */
   	    lcour=alloue_liste_point(1);
     	    lcour->x=x;  lcour->y=y;  lcour->z=-999.999;
            lcour->statut=car; lcour->noinit=-(no2+1);
            lcour->suc=lc;  lcpred->suc=lcour;
            break;
	   }
    lcpred=lc; lc=lc->suc;  dist=0.;
   }
 /* liste_lcir_ecran(lcird); */
}
/*--------------------------------------------------------------*/
int statut_point(lcir,car,no)
struct liste_point *lcir;
char car;
int no;
{ struct liste_point *lcird;
    /*sprintf(imp,"\n STATUT POINT ");imprime();*/
    lcird=lcir;
    while(lcir->noinit!=no)lcir=lcir->suc;
    lcir->statut=car;
    if(lcir->suc==NULL)lcird->statut=car;
    else if(lcir==lcird){while(lcir->suc)lcir=lcir->suc;
		    lcir->statut=car;
                   }
  /*  liste_lcir_ecran(lcird); */
}
/*--------------------------------------------------------------*/
int intersecte(no1,no2)
int no1,no2;
{struct inters *li,*lci;
   /* ajoute une intersection faite dans la liste */

     if(no1==-1)no1=cir0.nbp-2;  else if(no1==cir0.nbp-1)no1=0;
     if(no2==-1)no2=cir1.nbp-2;  else if(no2==cir1.nbp-1)no2=0;
       

     /*sprintf(imp,"\n INTERSECTE no1 = %d no2 = %d ",no1,no2);imprime();*/
/*
    if(no1==cir0.nbp-1 && no2==cir1.nbp-1){sup_inter(0,0);sup_inter(lcir1,0,0);}
    else if(no1==cir0.nbp-1){sup_inter(lcir0,0,no2);sup_inter(lcir1,no2,0);}
    else if(no2==cir1.nbp-1){sup_inter(lcir1,0,no1);sup_inter(lcir0,no1,0);}
*/

	lci=alloue_inters(1);
        lci->noa1=no1;  lci->noa2=no2;
   
      if(debut_inter==NULL)debut_inter=lci;
      else
        {li=debut_inter;
         while(li->suc)li=li->suc; 
         li->suc=lci;
        }
     
     liste_intersecte();
}
/*--------------------------------------------------------------*/
int verif_intersecte(no1,no2)
int no1,no2;
{struct inters *li;
 struct liste_point *p0,*p1;

   if(no1<0 && no2<0){intersecte(cir0.nbp-2,cir1.nbp-2);return;}
   else if(no1<0){intersecte(cir0.nbp-2,no2);return;}
   else if(no2<0){intersecte(no1,cir1.nbp-2);return;}

   if(debut_inter==NULL )return;
   li=debut_inter;
   while(li)
        {if(li->noa1==no1 && li->noa2==no2)
             {/* on supprime les inters ajoutees sur les 2 circuits */
                p0=test_sup_inter(lcir0,no1,no2);  
                if(p0)
		  {p1=test_sup_inter(lcir1,no2,no1);
                   if(p1){sup_inter(lcir0,p0); sup_inter(lcir1,p1);}
		  }
 	        break;
	     }
         li=li->suc;
	}
}
/*--------------------------------------------------------------*/
int liste_intersecte()
{struct inters *li;
   /*sprintf(imp,"\n\n liste intersecte \n");imprime();*/
  if(debut_inter==NULL)
         {/*sprintf(imp,"\n liste vide ");imprime();*/return;}
  li=debut_inter;
   while(li)
    {/*sprintf(imp,"\n li = %ld noa1 = %d noa2 = %d  suc = %ld",li,li->noa1,li->noa2,li->suc);imprime();*/
     li=li->suc;
    }
}
/*--------------------------------------------------------------*/
struct liste_point *test_sup_inter(lcir,no1,no2)
struct liste_point *lcir;
int no1,no2;
{ struct liste_point *lc,*lcpred,*lcird;
  no2=-(no2+1);
   /*sprintf(imp,"\n supprime inter no1= %d no2 = %d ",no1,no2);imprime();*/
  lcird=lcir;  lc=lcir;  lcpred=NULL;
  while(lc!=NULL&&lc->noinit!=no1){lcpred=lc;  lc=lc->suc;}
  do  {lcpred=lc;  lc=lc->suc;}
  while(lc!=NULL&&lc->noinit!=no2&&lc->noinit<0);
  if(lc&&lc->noinit<0) return(lc);
  /* liste_lcir_ecran(lcird); */
 return(NULL);
}

/*--------------------------------------------------------------*/
int sup_inter(lcir,lcs)
struct liste_point *lcir,*lcs;
{ struct liste_point *lc,*lcpred,*lcird;
    lcird=lcir;  lc=lcir;  lcpred=NULL;
  while(lc!=lcs){lcpred=lc;  lc=lc->suc;}
       lcpred->suc=lc->suc; libere_liste_point(lc,1);	
}
/*--------------------------------------------------------------*/
int lit_intersecte(no1,no2)
int no1,no2;
{struct inters *li;
   if(debut_inter==NULL)return(0);
   li=debut_inter;
   while(li)
        {if(li->noa1==no1 && li->noa2==no2)return(1);
         li=li->suc;
        }

   return(0);
}

/*--------------------------------------------------------------*/
struct liste_point *lcopie(cir)
struct circuit *cir;
{struct liste_point *lc,*lcir;
 int k;
 lcir=alloue_liste_point(2);
 lc=lcir;
 for(k=0;k<cir->nbp;k++)
     { if(k){lc->suc=alloue_liste_point(2); 
             lc=lc->suc; 
            }
       lc->suc=NULL;
       lc->statut='q';   lc->noinit=k;
       lc->x=cir->x[k];  lc->y=cir->y[k]; lc->z=cir->z[k];
     }
 return(lcir);
}

/******************************************************************/
/* singul.c */
/* methode des singularites pour operation entre polygones */
/* avec info statuts points , statuts aretes */
/* et des coord  sans autre signification que pt commun ou inter */
/*      affectees par le programme */


/******************************************************************/
struct spol *copie_lcir_spol(lcir)
struct liste_point *lcir;
{int nbp,i; 
 struct spol *poly;
  nbp=nbp_lcir(lcir);
  poly=alloue_poly(nbp,1);
  poly->nbp=nbp;
  i=0;
  while(lcir)
    {poly->x[i]=lcir->x; poly->y[i]=lcir->y; poly->z[i]=lcir->z;
     poly->statut_point[i]=lcir->statut;
     poly->statut_arete[i]=' '; poly->traite_arete[i]=' ';
     lcir=lcir->suc; i++;
    }
 return(poly);
}
/******************************************************************/
int nbp_lcir(lc)
struct liste_point *lc;
{int i;
  i=0;
  while(lc){i++; lc=lc->suc;}
  return(i);
}
/******************************************************************/
int z_cir1()
{struct liste_point *lc;
  if(option_calcul_z)
    { lc=lcir1;
      while(lc)
          {lc->z=-999.999;
           lc=lc->suc;
          }
    }
}

/******************************************************************/
int initialise_statut_point(type,poly)
char type;
struct spol *poly;
{ int i;
   for(i=0;i<poly->nbp-1;i++)  poly->statut_point[i]=type;
}
/******************************************************************/
int typage_des_aretes_communes(poly1,poly2)
struct spol *poly1,*poly2;
{
  int i,j,k;

  for(i=0;i<poly1->nbp-1;i++)
    { if(poly1->statut_point[i]=='c' && poly1->statut_point[i+1]=='c')
        {        /* chercher 2 pts consecutifs de statuts C */
                 /* et identiques ds poly2 */
           for(j=0;j<poly2->nbp-1;j++)
             { if(poly2->statut_arete[j]!='c')
                   { if(poly2->statut_point[j]=='c' && poly2->statut_point[j+1]=='c')
                       {   /* test pts i,i+1 de poly1 et j,j+1 de poly2 */
			k=0;
                         if(poly1->x[i]==poly2->x[j] && poly1->y[i]==poly2->y[j])
                            { if(poly1->x[i+1]==poly2->x[j+1] && poly1->y[i+1]==poly2->y[j+1]) k=1;
                            }
                          else if (poly1->x[i]==poly2->x[j+1] && poly1->y[i]==poly2->y[j+1])
                            { if(poly1->x[i+1]==poly2->x[j] && poly1->y[i+1]==poly2->y[j]) k=1;                                       }
                          if(k)
                            {      /* ok arete de TYPE C */
                               poly1->statut_arete[i]='c';
                               poly2->statut_arete[j]='c';
                               break;  /* abandonne recherche ds poly2 */
                            }
                       }  
                   }
             }
        }        
    }
}
 
/******************************************************************/
int typage_des_aretes_int_ext(poly1,poly2)
struct spol *poly1,*poly2;
{
  char statut;  /* I,E,C ou Non connu */
  int i;
              /* typage des aretes Interieur ou Exterieur */
  statut='N';
  for(i=0;i<poly1->nbp-1;i++)
    { if(poly1->statut_arete[i]!='c')
        {
          if(poly1->statut_point[i]=='q')
             {  if(statut=='N') 
		    {statut=calcul_statut(i,poly1,poly2);
		     if(statut=='w')return(999);
		    }
             }
          else if( poly1->statut_point[i]=='i')
             { if(statut=='i') statut='e'; else statut='i';
             }
          else /* statut_point[i]=='c' */
             { statut=calcul_statut(i,poly1,poly2);
             }
          poly1->statut_arete[i]=statut; 
        }
    }
}

/******************************************************************/
char calcul_statut(no,poly1,poly2)
int no;
struct spol *poly1,*poly2;
{          /* ATTENTION : en fait inpoly() */
          /* sur milieu arete no de poly1 par rapport a poly2 */
 double xm,ym; 
 int idedan,isur;
 struct circuit *cir;
   xm=(poly1->x[no]+poly1->x[no+1])/2;
   ym=(poly1->y[no]+poly1->y[no+1])/2;
  
   cir=copy_poly_cir(poly2);

   inpoly(&idedan,&isur,xm,ym,cir);
   desalloue_circuit(cir,98);

   /* si sur contour pb , signaler et faire exit */
   if(isur==1){printf("\n PB calcul_statut sur contour entre faces %d et %d\n",traite_face1,traite_face2);
		printf(" no du pt = %d x = %lf y = %lf ",no,xm,ym);
                return('w');
	       /*
                renseigne(poly1,poly2);
                debut_inter=NULL;
                lcir0=lcopie(&cir0); lcir1=lcopie(&cir1);
                intcor1();
                liste_lcir_ecran(lcir0); liste_lcir_ecran(lcir1);
                exit(0);
		*/
               /* return('c'); *//* on suppose */
              }
   if(idedan==1)return('i');
   else return('e');
  
}
/******************************************************************/
struct circuit *copy_poly_cir(poly)
struct spol *poly;
{int i; 
struct circuit *cir;
   cir=alloue_circuit(97);
   cir->nbp=poly->nbp;
   alloue_point_circuit(cir,97);
   for(i=0;i<poly->nbp;i++)
	{cir->x[i]=poly->x[i]; cir->y[i]=poly->y[i]; cir->z[i]=poly->z[i];}
   return(cir);
}
/******************************************************************/
struct spol *copy_poly_poly(poly)
struct spol *poly;
{int i; 
struct spol *polyc;
   polyc=alloue_poly(poly->nbp,2);
   polyc->nbp=poly->nbp;
   for(i=0;i<poly->nbp;i++)
     {polyc->x[i]=poly->x[i]; polyc->y[i]=poly->y[i]; polyc->z[i]=poly->z[i];}
   return(polyc);
}


/******************************************************************/
int quelle_methode(poly)
struct spol *poly;
{ int i;
     /* pour la methode NON SINGULARITE ... */
     /* il ne faut pas de pts de type commun */
  for(i=0;i<poly->nbp-1;i++)
   { if(poly->statut_point[i]=='c') { methode_non_singul=0; return; }
   }
     /* il faut des pts de type intersection */
  for(i=0;i<poly->nbp-1;i++)
   { if(poly->statut_point[i]=='i') { methode_non_singul=1; return; }
   }
  methode_non_singul=0;
}

/******************************************************************/
int poly_inter_poly(poly1,poly2,pcont)
struct spol *poly1,*poly2;
struct contour *pcont;
{
  int i,j;
  struct spol *poly_resul;
  int idedan,isur;
  struct circuit *cir;
       /* INTERSECTION poly1 inter poly2 */
 if(methode_non_singul==0)
  {
   j=poly_disjoint_inclus_egal(poly1,poly2);
      /* cas particuliers */
   if(j==0 || j==5)return(-1);
   if(j==1 || j==6)return(1);
   if(j==2 || j==7 || j==3)return(2);
  } 
     /* a determiner j=4 */

 if(methode_non_singul==0)
     {     /* METHODE SINGULARITE */
       i=0;
        while((j=cherche_segment_du_type('i',poly1))!=-1)
             {  i++; /* printf("Polygone %d\n",i);*/
                poly_resul=alloue_poly(poly1->nbp+poly2->nbp,3);
                if(construit_poly_inter(poly1,j,poly2,poly_resul)==999)return(999);
                epure_polygone_resultat(poly_resul,pcont);
             }
      } 
  else 
      {     /* METHODE NON SINGULARITE */
            /* cherche si pt 1 de poly1 ds poly2 */
       cir=copy_poly_cir(poly2);
       inpoly(&idedan,&isur,poly1->x[0],poly1->y[0],cir);
       desalloue_circuit(cir,1290);
       if(idedan==1)
           { /* demarre sur poly2 */
             j=cherche_no_inter(poly1); poly1->traite_arete[j]='N';
             /* considere pour utilisation standard de cherche_xy_ds_poly */
             /* un pt ds poly_resul */
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,4);
             place_point_ds_poly_resultat(j,poly1,poly_resul,0);
             /* cherche debut du parcours sur poly2 */
             j=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
             poly_resul->nbp=0;
             if(construit_poly_operation_nonsingul(poly2,1,j,poly1,1,poly_resul)==999)return(999);
             epure_polygone_resultat(poly_resul,pcont);
          }
       while((j=cherche_no_inter(poly1))!=-1)  /* cherche inter non traitee */
          {
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,5);
             if(construit_poly_operation_nonsingul(poly1,1,j,poly2,1,poly_resul)==999)return(999);
             epure_polygone_resultat(poly_resul,pcont);
          }
     }
    if(pcont->debut_support==NULL)return(-1); else return(0);
}

/******************************************************************/
int construit_poly_inter(poly1,no,poly2,poly_resul)
struct spol *poly1,*poly2,*poly_resul;
int no;
{
     /*  printf(" Sur Poly 1 :\n"); */
   place_point_ds_poly_resultat(no,poly1,poly_resul,0);
   while(1)
      { if(parcours_arete_du_type('i',poly1,no,1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return(1);
          /*  printf(" Sur Poly 2 :\n");*/
        no=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
        if(no==999)return(999);
        if(parcours_arete_du_type('i',poly2,no,1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return;
        no=cherche_xy_ds_poly(poly1,poly_resul,poly1,poly2);
        if(no==999)return(999);
          /*  printf(" Sur Poly 1 :\n");*/
      } 
}

/******************************************************************/
int poly_moins_poly(poly1,poly2,pcont)
struct spol *poly1,*poly2;
struct contour *pcont;
{
  int i,j;
  struct spol *poly_resul,*polyc;
  int idedan,isur;
  struct circuit *cir;
  struct contour *pcont1;

       /* DIFFERENCE poly1 moins poly2 */
 if(methode_non_singul==0)
  {
   j=poly_disjoint_inclus_egal(poly1,poly2);
       /* cas particuliers */
   if(j==0 || j==5)return(-1);  
   if(j==1)return(1);
   if(j==2 || j==7 || j==3)return(2);
  }
       /* a determiner  j=4  ou 6 */

 if(methode_non_singul==0)
     {     /* METHODE SINGULARITE */
       /*printf(i"RESULTAT MOINS\n"); */
       i=0; 
        while((j=cherche_segment_du_type('e',poly1))!=-1)
              { i++;/* printf("Polygone %d\n",i);*/
                poly_resul=alloue_poly(poly1->nbp+poly2->nbp,6);
                if(construit_poly_moins(poly1,j,poly2,poly_resul)==999)return(999);
	        pcont1=alloue_contour(13); polyc=copy_poly_poly(poly_resul);
                epure_polygone_resultat(polyc,pcont1);
	        met_resultat_pcont(poly_resul,pcont1,pcont);
              }
      }
  else 
      {     /* METHODE NON SINGULARITE */
            /* cherche si pt 1 de poly1 ds poly2 */
       cir=copy_poly_cir(poly2);
       inpoly(&idedan,&isur,poly1->x[0],poly1->y[0],cir);
       desalloue_circuit(cir,1290);
       if(idedan!=1)
           { /* demarre sur poly2 */
             j=cherche_no_inter(poly1); poly1->traite_arete[j]='N';
             /* considere pour utilisation standard de cherche_xy_ds_poly */
             /* un pt ds poly_resul */
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,7);
             place_point_ds_poly_resultat(j,poly1,poly_resul,0);
             /* cherche debut du parcours sur poly2 */
             j=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
             poly_resul->nbp=0;
             if(construit_poly_operation_nonsingul(poly2,-1,j,poly1,1,poly_resul)==999)return(999);
 	    pcont1=alloue_contour(13); polyc=copy_poly_poly(poly_resul);

            epure_polygone_resultat(polyc,pcont1);
	     met_resultat_pcont(poly_resul,pcont1,pcont);
          }
       while((j=cherche_no_inter(poly1))!=-1)  /* cherche inter non traitee */
          {
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,8);
             if(construit_poly_operation_nonsingul(poly1,1,j,poly2,-1,poly_resul)==999)return(999);
	     pcont1=alloue_contour(13); polyc=copy_poly_poly(poly_resul);

             epure_polygone_resultat(polyc,pcont1);
	     met_resultat_pcont(poly_resul,pcont1,pcont);
          }
        }
   if(pcont->debut_support==NULL)return(2); else return(0);
}
/******************************************************************/
int met_resultat_pcont(poly,pcont1,pcont)
struct contour *pcont,*pcont1;
struct spol *poly;
{ struct circuit *pcir,*cir;
  int i,test; 
    pcir=pcont1->debut_support;
    test=0;
    while(pcir)
	{if(surface(pcir->nbp,pcir->x,pcir->y)>0)test=1;
	 pcir=pcir->suc;
	}
         if(test==1)
	     {/* on met poly_resul a la suite de pcont */
              if(poly->nbp>3)
                      {cir=copy_poly_cir(poly);
                       if(option_sup_surf)sup_surf_circuit(cir);	                                   if(cir->nbp>3)
                           {for(i=0;i<4;i++)cir->vnorm[i]=cir0.vnorm[i];
                            for(i=0;i<cir->nbp-1;i++)
			      {if(cir->z[i]==-999.999)
                                 cir->z[i]=calcul_zplan(&cir0,cir->x[i],cir->y[i]);
			      }
                            cir->z[cir->nbp-1]=cir->z[0];
                            fenetre_circuit(cir);
                            ajoute_cir_pcont(cir,pcont);
                           }
                        else desalloue_circuit(cir,744);
                       }
	     desalloue_contour(pcont1,100);
            }
         else
            {/* on met pcont1 a la suite de pcont */
              if(pcont->debut_support==NULL) 
                     pcont->debut_support=pcont1->debut_support;
	      else {cir=pcont->debut_support;
	            while(cir->suc!=NULL)cir=cir->suc;
		    cir->suc=pcont1->debut_support;
		   }
	      libere_contour(pcont1,1);
            }
 desalloue_poly(poly);
}

/******************************************************************/
int construit_poly_moins(poly1,no,poly2,poly_resul)
struct spol *poly1,*poly2,*poly_resul;
int no;
{
      /*printf(" Sur Poly 1 :\n");*/
   place_point_ds_poly_resultat(no,poly1,poly_resul,0);
   while(1)
      { if(parcours_arete_du_type('e',poly1,no,1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return;
            /*printf(" Sur Poly 2 :\n");*/
        no=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
	if(no==999)return(999);
        if(parcours_arete_du_type('i',poly2,no,-1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return;
        no=cherche_xy_ds_poly(poly1,poly_resul,poly1,poly2);
	if(no==999)return(999);
            /*printf(" Sur Poly 1 :\n");*/
      } 
}

/******************************************************************/
int poly_union_poly(poly1,poly2,pcont)
struct spol *poly1,*poly2;
struct contour *pcont;
{
  int i,j;
  struct spol *poly_resul;
  int idedan,isur;
  struct circuit *cir;


       /* UNION poly1 moins poly2 */
 if(methode_non_singul==0)
  {
   j=poly_disjoint_inclus_egal(poly1,poly2);
       /* cas particuliers */
   if(j==0) return(-1); 
   if(j==1 || j==6)return(1);
   if(j==2|| j==7)return(2);
   if(j==3)return(1);
  }
       /* a determiner  j=4 ou 5 */

 if(methode_non_singul==0)
     {     /* METHODE SINGULARITE */
           /* si plusieurs polys , les polys TROUS sont obligatoirement */
           /* de SENS inverse et Inclus ds un poly (resultat de Union)  */
        /*sprintf(imp,"RESULTAT UNION\n");imprime();*/ i=0;
        while((j=cherche_segment_du_type('e',poly1))!=-1)
              { i++; /*sprintf(imp,"Polygone %d\n",i);imprime();*/
                poly_resul=alloue_poly(poly1->nbp+poly2->nbp,9);
                if(construit_poly_union(poly1,j,poly2,poly_resul)==999)return(999);
                epure_polygone_resultat(poly_resul,pcont);
              }
      }
  else 
      {     /* METHODE NON SINGULARITE */
            /* cherche si pt 1 de poly1 ds poly2 */
       cir=copy_poly_cir(poly2);
       inpoly(&idedan,&isur,poly1->x[0],poly1->y[0],cir);
       desalloue_circuit(cir,1290);
       if(idedan!=1)
           { /* demarre sur poly2 */
             j=cherche_no_inter(poly1); poly1->traite_arete[j]='N';
             /* considere pour utilisation standard de cherche_xy_ds_poly */
             /* un pt ds poly_resul */
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,10);
             place_point_ds_poly_resultat(j,poly1,poly_resul,0);
             /* cherche debut du parcours sur poly2 */
             j=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
             poly_resul->nbp=0;
             if(construit_poly_operation_nonsingul(poly2,1,j,poly1,1,poly_resul)==999)return(999);
             epure_polygone_resultat(poly_resul,pcont);
          }
       while((j=cherche_no_inter(poly1))!=-1)  /* cherche inter non traitee */
          {
/*printf("demarre sur poly1 ;no pt inter=%d\n",j);*/
             poly_resul=alloue_poly(poly1->nbp+poly2->nbp,11);
             if(construit_poly_operation_nonsingul(poly1,1,j,poly2,1,poly_resul)==999)return(999);
             epure_polygone_resultat(poly_resul,pcont);
          }
        }
   if(pcont->debut_support==NULL)return(-1); else return(0);
}

/******************************************************************/
int construit_poly_union(poly1,no,poly2,poly_resul)
struct spol *poly1,*poly2,*poly_resul;
int no;
{
      /*sprintf(imp," Sur Poly 1 :\n"); imprime();*/
   place_point_ds_poly_resultat(no,poly1,poly_resul,0);
   while(1)
      { if(parcours_arete_du_type('e',poly1,no,1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return;
            /*sprintf(imp," Sur Poly 2 :\n");imprime();*/
        no=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
	if(no==999)return(999);
        if(parcours_arete_du_type('e',poly2,no,1,poly_resul)==999)return(999);
        if((test_fin_parcours(poly_resul))) return;
        no=cherche_xy_ds_poly(poly1,poly_resul,poly1,poly2);
 	if(no==999)return(999);
           /*sprintf(imp," Sur Poly 1 :\n"); imprime();*/
      } 
}

/******************************************************************/
int construit_poly_operation_nonsingul(poly1,sens1,no,poly2,sens2,poly_resul)
struct spol *poly1,*poly2,*poly_resul;
int sens1,no,sens2;
{  /* pour METHODE NON SINGULARITE */
   place_point_ds_poly_resultat(no,poly1,poly_resul,0);
   while(1)
     { parcours_polygone_nonsingul(poly1,sens1,no,poly_resul);
      if((test_fin_parcours(poly_resul))) return;
       no=cherche_xy_ds_poly(poly2,poly_resul,poly1,poly2);
       if(no==999)return(999);
       parcours_polygone_nonsingul(poly2,sens2,no,poly_resul);
       if((test_fin_parcours(poly_resul))) return;
       no=cherche_xy_ds_poly(poly1,poly_resul,poly1,poly2);
       if(no==999)return(999);
     }
}

/******************************************************************/
int parcours_polygone_nonsingul(poly,sens,no,poly_resul)
struct spol *poly,*poly_resul;
int no,sens;
{
 int no2;
 poly->traite_arete[no]='N';
 if(sens==1)
    { do
        {     /* prend point no++ */
          no++; if(no+1==poly->nbp)no=0;
          place_point_ds_poly_resultat(no,poly,poly_resul,0);
        } while(poly->statut_point[no]!='i');
    }
 else
    { do
        {     /* prend point no-- */
          no--; if(no<0)no=poly->nbp-2;
          place_point_ds_poly_resultat(no,poly,poly_resul,0);
        } while(poly->statut_point[no]!='i');
    }
 poly->traite_arete[no]='N';
}


/******************************************************************/
int poly_disjoint_inclus_egal(poly1,poly2)
struct spol *poly1,*poly2;
{
 int i,j,e1,c1,i1,e2,c2,i2;
       /* retourne 0 : disjoint ; que des statuts E             */
       /*          5 : disjoint mitoyen ; que des statuts E et C*/
       /*          1 : poly1 Inclus poly2 ; que statut E  poly1 */
       /*                                et que statut I  poly2 */
       /*          6 : poly1 Inclus poly2 ; statut E et C poly1 */
       /*                                et statut I et C poly2 */
       /*          2 : poly2 Inclus poly1 ; que statut E  poly2 */
       /*                                et que statut I  poly1 */
       /*          7 : poly2 Inclus poly1 ; statut E et C poly2 */
       /*                                et statut I et C poly1 */
       /*          3 : poly1==poly2 ; que des statuts C         */
       /*          4 : en intersection                          */
  e1=0; c1=0; i1=0;
  for(i=0;i<poly1->nbp-1;i++)
    { if(poly1->statut_arete[i]=='e') e1=1;
      if(poly1->statut_arete[i]=='c') c1=1;
      if(poly1->statut_arete[i]=='i') i1=1;
    }
  e2=0; c2=0; i2=0;
  for(i=0;i<poly2->nbp-1;i++)
    { if(poly2->statut_arete[i]=='e') e2=1;
      if(poly2->statut_arete[i]=='c') c2=1;
      if(poly2->statut_arete[i]=='i') i2=1;
    }
  if(e1+i1+e2+i2==0) return(3);
  if(i1+i2==0)  { if(c1==0) return(0); else return(5); }
  if(i1==0 && e2==0) { if(c1==0) return(1); else return(6); }
  if(e1==0 && i2==0) { if(c1==0) return(2); else return(7); }
  return(4);
}

/******************************************************************/
int cherche_segment_du_type(type,poly)
char type;
struct spol *poly;
{
 int i;
        /* renvoie indice du pt du poly d arete du type non traite */
 for(i=0;i<poly->nbp-1;i++)
   { if(poly->statut_arete[i]==type && poly->traite_arete[i]=='O') return(i);
   }
 return(-1);
}

/******************************************************************/
int cherche_no_inter(poly)
struct spol *poly;
{ int i;
        /* renvoie indice du pt du poly de type Inter non traite */
 for(i=0;i<poly->nbp-1;i++)
   { if(poly->statut_point[i]=='i' && poly->traite_arete[i]=='O') return(i);
   }
 return(-1);
}

/******************************************************************/
int test_fin_parcours(poly)
struct spol *poly;
{    /* test si poly resul ferme : 1er pt= dernier pt  ;return(1) */

  if(poly->x[0]==poly->x[poly->nbp-1] && poly->y[0]==poly->y[poly->nbp-1])return(1);
  return(0);
}

/******************************************************************/
int cherche_xy_ds_poly(poly,poly_resul,poly1,poly2)
struct spol *poly,*poly_resul;
{
 int i;
        /* cherche ds poly le pt identique au dernier pt de poly_resul */
        /* renvoie indice du pt du poly  */
 for(i=0;i<poly->nbp-1;i++)
   { if(poly->x[i]==poly_resul->x[poly_resul->nbp-1] && poly->y[i]==poly_resul->y[poly_resul->nbp-1]) return(i);
   }
       /* doit trouver un pt */
 printf("\n PB cherche_xy_ds_poly entre faces %d et %d\n",traite_face1,traite_face2);
 renseigne(poly1,poly2);
/*
 debut_inter=NULL;
   lcir0=lcopie(&cir0); lcir1=lcopie(&cir1);

 intcor1();
liste_lcir_ecran(lcir0); liste_lcir_ecran(lcir1);
 exit(0);
*/
return(999);
}

/******************************************************************/
int parcours_arete_du_type(type,poly,no,sens,poly_resul)
char type;
struct spol *poly,*poly_resul;
int no,sens;
{
 int no2;
       /* le 1er pt no est pris sans test du statut arete */
 if(sens==1)
    { do
        {     /* traite arete no  x[no],y[no] et x[no+1],y[no+1] */
          if(poly->traite_arete[no]=='N'){printf("\n pb pt commun ds circuit \n");return(999);}
          poly->traite_arete[no]='N';
          no++; if(no+1==poly->nbp)no=0;
          place_point_ds_poly_resultat(no,poly,poly_resul,0);
        } while(poly->traite_arete[no]!='N' && poly->statut_arete[no]==type);
    }
 else
    { do
        {     /* prend arete no  x[no],y[no] et x[no-1],y[no-1] */
          no--; if(no<0)no=poly->nbp-2;
          if(poly->traite_arete[no]=='N'){printf("\n pb pt commun ds circuit \n");return(999);}
         poly->traite_arete[no]='N';
          place_point_ds_poly_resultat(no,poly,poly_resul,0);
          no2=no-1; if(no2<0)no2=poly->nbp-2;
        } while(poly->traite_arete[no2]!='N' && poly->statut_arete[no2]==type);
    }
}

/******************************************************************/
int place_point_ds_poly_resultat(no,poly,poly_resul,impr)
int no,impr;
struct spol *poly,*poly_resul;
{
      /* prend le pt no du poly */
 /* printf(" prend point %2d : %5.2f %5.2f\n",no+1,poly->x[no],poly->y[no]);*/
  
  poly_resul->x[poly_resul->nbp]=poly->x[no];
  poly_resul->y[poly_resul->nbp]=poly->y[no];
  poly_resul->z[poly_resul->nbp]=poly->z[no];
  poly_resul->nbp++;
}

/******************************************************************/
int epure_polygone_resultat(poly,pcont)
struct spol *poly;
struct contour *pcont;
{ struct circuit *cir;
  struct spol *poly_dec;
  int i;
        /* epure le poly des pts communs */
        /* procedure RECURSIVE qui constitue 2 polys a partir */
        /* du poly initial avec 2 pts communs */
  if(recherche_pt_commun_ds_poly(poly))
    {  poly_dec=alloue_poly(poly->nbp,12);
       constitue_poly_decouple(poly,poly_dec,1);
       epure_polygone_resultat(poly_dec,pcont);  /* recursif */
       poly_dec=alloue_poly(poly->nbp,13);
       constitue_poly_decouple(poly,poly_dec,2);
       epure_polygone_resultat(poly_dec,pcont);  /* recursif */
       desalloue_poly(poly);
    }
  else
    {     /* imprime POLY RESULTAT et desalloue */
       if(poly->nbp>3){/*printf("\n   POLY RESULTAT :\n");*/
                       /*imprime_poly_decouple(poly);*/
		         cir=copy_poly_cir(poly);
                        if(option_sup_surf)sup_surf_circuit(cir);	                                if(cir->nbp>3)
                           {for(i=0;i<4;i++)cir->vnorm[i]=cir0.vnorm[i];
                           for(i=0;i<cir->nbp-1;i++)
			      {if(cir->z[i]==-999.999)
                                 cir->z[i]=calcul_zplan(&cir0,cir->x[i],cir->y[i]);
			      }
                            cir->z[cir->nbp-1]=cir->z[0];
                            fenetre_circuit(cir);
                            ajoute_cir_pcont(cir,pcont);
                           }
                         else desalloue_circuit(cir,745);
                       }
       desalloue_poly(poly);
    }
}  
/******************************************************************/
int ajoute_cir_pcont(cir,pcont)
struct circuit *cir;
struct contour *pcont;
{ struct circuit *circ;
  if(pcont->debut_support==NULL){pcont->debut_support=cir;cir->suc=NULL;}
  else {/* rajoute au debut */
	cir->suc=pcont->debut_support; 
        pcont->debut_support=cir;
       }
}
/******************************************************************/
int recherche_pt_commun_ds_poly(poly)
struct spol *poly;
{
  int i,j;
      /* recherche 2 pts communs ds poly  et met statut c entre les 2 */
  initialise_statut_point(' ',poly);
  for(i=0;i<poly->nbp-1;i++)
    { for(j=i+1;j<poly->nbp-1;j++)
        { if(fabs(poly->x[i]-poly->x[j])<DISCOL && fabs(poly->y[i]-poly->y[j])<DISCOL)
             { /*liste_poly(poly);*/
	       poly->statut_point[i]='c'; poly->statut_point[j]='c';
	       poly->x[i]=poly->x[j]; poly->y[i]=poly->y[j];
              /* printf(" un point commun en %d et %d\n",i+1,j+1);*/
               return(1);
             }
        } 
    } 
  return(0); 
}

/******************************************************************/
int constitue_poly_decouple(poly,poly_dec,id)
struct spol *poly,*poly_dec;
int id;
{
  int k,no;
      /* cherche le 1er ou 2eme (suivant id=1 ou 2) pt de statut 'c' */
  k=0;
  for(no=0;no<poly->nbp;no++)
     { if(poly->statut_point[no]=='c')
          { k++; if(k==id) break; }
     }
       /* constitue poly_dec du pt 'c' au pt 'c' de poly */
   do
     {  place_point_ds_poly_resultat(no,poly,poly_dec,0);
        no++; if(no+1==poly->nbp)no=0;                
     } while(poly->statut_point[no]!='c');
   place_point_ds_poly_resultat(no,poly,poly_dec,0);
 ;
}

/******************************************************************/
int imprime_poly_decouple(poly)
struct spol *poly;
{
  int i;
  if(poly->nbp<4) { printf("     moins de 3 pts\n"); return(0); }
  else
    {
     for(i=0;i<poly->nbp;i++)
        {printf("          %2d - %lf %lf\n",i+1,poly->x[i],poly->y[i]);} 
     
    }
}
/******************************************************************/
int renseigne(poly1,poly2)
struct spol *poly1,*poly2;
{int i;
   return;
   /*sprintf(imp,"\n LISTE DE CIR0 ");imprime();*/
   liste_circuit_ecran(&cir1);
   /*sprintf(imp,"\n LISTE DE CIR1 ");imprime();*/
   liste_circuit_ecran(&cir0);

   printf("\n RESULTAT DU TYPAGE ");
  /* Resultat du typage */
   printf("\nPOLY 1  nb pts = %d\n",poly1->nbp);
   for(i=0;i<poly1->nbp;i++)
        {printf("   Statut Pt %2d :  %c  %c  %5.2lf %5.2lf\n",i+1,poly1->statut_point[i],poly1->statut_arete[i],poly1->x[i],poly1->y[i]);
        }
   printf("\nPOLY 2  nb pts = %d\n",poly2->nbp);
   for(i=0;i<poly2->nbp;i++)
       {printf("   Statut Pt %2d :  %c  %c  %5.2lf %5.2lf\n",i+1,poly2->statut_point[i],poly2->statut_arete[i],poly2->x[i],poly2->y[i]);
       }
/*
   printf("\n apres surface ");
    sup_surf_circuit(&cir0);  sup_surf_circuit(&cir1);
 printf("\n LISTE DE CIR0 ");
   liste_circuit_ecran(&cir1);
   printf("\n LISTE DE CIR1 ");
   liste_circuit_ecran(&cir0);
*/

}

/*--------------------------------------------------------------*/

int intcor1()
{int i,j,ii,jj;
 double xint,yint,t1,t2;
      
    for(j=0;j<cir0.nbp-1;j++)
       {
      printf("\n\n ****** cherche inter cir0 arete %d *\n",j);
      
         /* fenetre arete j de cir0 */
	         fen_arete(&cir0,j,fen);

        for(i=0;i<cir1.nbp-1;i++)
	 { /* verifie si l'intersection est a calculer */ 
               jj=lit_intersecte1(j,i);
         
            printf("\n                     et cir1 arete %d",i); 
            if(jj){printf(" inter a ne pas traitee\n  ");}
           if(jj==0)
             {/* fenetre arete i de cir1 */
  	         fen_arete(&cir1,i,fen+1);
	      /* test si les fenetres des 2 aretes se coupent */
	  if(recoup(fen,fen+1)!=-1)
	   {/* cherche inter */ 
	      ii=inter(&cir0.x[j],&cir0.y[j],&cir1.x[i],&cir1.y[i],&xint,&yint,&t1,&t2);
	      if(ii==1)
	      {
               printf("t1 = %lf t2 = %lf xint = %lf yint = %lf\n ",t1,t2,xint,yint);

		intersecte1(j,i);

               if(t1!=0 && t1!=1 && t2!=0 && t2!=1)
		      {/* c'est une intersection franche */
		        ajout_point1(lcir0,'i',j,i,xint,yint);
			ajout_point1(lcir1,'i',i,j,xint,yint);
               	      }
                else if(t1==0 && t2==0)
		      {verif_intersecte1(j-1,i);
		       verif_intersecte1(j,i-1);
		       verif_intersecte1(j-1,i-1);
		       statut_point1(lcir0,'c',j);
		       statut_point1(lcir1,'c',i);
		      }
                else if(t1==0 && t2!=0 && t2!=1)
		      {verif_intersecte1(j-1,i);
	               ajout_point1(lcir1,'c',i,j,cir0.x[j],cir0.y[j]);
		       statut_point1(lcir0,'c',j);
		      }
                else if(t2==0 && t1!=0 && t1!=1)
		      {verif_intersecte1(j,i-1); 
                      ajout_point1(lcir0,'c',j,i,cir1.x[i],cir1.y[i]);
		       statut_point1(lcir1,'c',i);
                      }
                else if(t1==0 && t2==1)
		     {verif_intersecte1(j-1,i);
		      verif_intersecte1(j-1,i+1); 
                      verif_intersecte1(j,i+1);
		      intersecte1(j,i+1);
		      statut_point1(lcir0,'c',j);
		      statut_point1(lcir1,'c',i+1);
                     }
                else if(t1==1 && t2==0)
		     {verif_intersecte1(j,i-1);
                      intersecte1(j+1,i);  intersecte1(j+1,i-1);  
                      statut_point1(lcir0,'c',j+1);
		      statut_point1(lcir1,'c',i);
                     }
		else if(t1==1 && t2==1)
		     {intersecte1(j+1,i);  intersecte1(j+1,i+1);
                      if(i==cir1.nbp-2)verif_intersecte1(j,0);
                      else intersecte1(j,i+1);
                      statut_point1(lcir0,'c',j+1);
		      statut_point1(lcir1,'c',i+1);  
                     }
	        else if(t1==1 && t2!=0 && t2!=1)
		     {if(j==cir0.nbp-2)verif_intersecte1(0,i);
                      else intersecte(j+1,i);
                      intersecte1(j+1,i); 
                      ajout_point1(lcir1,'c',i,j,cir0.x[j+1],cir0.y[j+1]);
		      statut_point1(lcir0,'c',j+1);
                     }	 
		else if(t2==1 && t1!=0 && t1!=1)
		     {if(i==cir1.nbp-2)verif_intersecte1(j,0);
                      else intersecte(j,i+1); 
                      /* pour pb */ intersecte(j,i);
		      ajout_point1(lcir0,'c',j,i,cir1.x[i+1],cir1.y[i+1]);
		      statut_point1(lcir1,'c',i+1);
                     }
	     }
           }
          }
	 }
       }
}
/*--------------------------------------------------------------*/
int ajout_point1(lcir,car,no1,no2,x,y)
struct liste_point *lcir;
char car;
int no1,no2;
double x,y;
{struct liste_point *lc,*lcour,*lcpred,*lcird;
 double dist;
 printf("  AJOUT POINT no1 = %d no2 = %d \n",no1,no2);
 dist=0.; lcird=lcir;
 while(lcir->noinit!=no1){printf(" lcir = %ld noinit = %d \n",lcir,lcir->noinit);lcir=lcir->suc;}
printf(" lcir = %ld \n",lcir);
  lcpred=lcir;
  lc=lcir->suc;
  while(1)
   {if(lc->noinit<0)
	{dist=test_distance(lcir->x,lcir->y,lc->x,lc->y,x,y);}
         printf(" dist = %lf\n ",dist);
         if(dist>0 || lc->noinit>=0)
	   {/* insere l'intersection */
   	    lcour=alloue_liste_point(3);
     	    lcour->x=x;  lcour->y=y;  lcour->z=-999.999;
            lcour->statut=car; lcour->noinit=-(no2+1);
            lcour->suc=lc;  lcpred->suc=lcour;
            break;
	   }
    lcpred=lc; lc=lc->suc;  dist=0.;
   }
 liste_lcir_ecran(lcird); 
}
/*--------------------------------------------------------------*/
int statut_point1(lcir,car,no)
struct liste_point *lcir;
char car;
int no;
{ struct liste_point *lcird;
    printf(" STATUT POINT \n");
    lcird=lcir;
    while(lcir->noinit!=no)lcir=lcir->suc;
    lcir->statut=car;
    if(lcir->suc==NULL)lcird->statut=car;
    else if(lcir==lcird){while(lcir->suc)lcir=lcir->suc;
		    lcir->statut=car;
                   }
  /*  liste_lcir_ecran(lcird); */
}
/*--------------------------------------------------------------*/
int intersecte1(no1,no2)
int no1,no2;
{struct inters *li,*lci;
   /* ajoute une intersection faite dans la liste */

     if(no1==-1)no1=cir0.nbp-2;  else if(no1==cir0.nbp-1)no1=0;
     if(no2==-1)no2=cir1.nbp-2;  else if(no2==cir1.nbp-1)no2=0;
       

     printf(" INTERSECTE no1 = %d no2 = %d \n",no1,no2);
/*
    if(no1==cir0.nbp-1 && no2==cir1.nbp-1){sup_inter1(0,0);sup_inter1(lcir1,0,0);}
    else if(no1==cir0.nbp-1){sup_inter1(lcir0,0,no2);sup_inter1(lcir1,no2,0);}
    else if(no2==cir1.nbp-1){sup_inter1(lcir1,0,no1);sup_inter1(lcir0,no1,0);}
*/
      lci=alloue_inters(2);
      lci->noa1=no1;  lci->noa2=no2;
   
      if(debut_inter==NULL)debut_inter=lci;
      else
        {li=debut_inter;
         while(li->suc)li=li->suc; 
         li->suc=lci;
        }
     
     liste_intersecte();
}
/*--------------------------------------------------------------*/
int verif_intersecte1(no1,no2)
int no1,no2;
{struct inters *li;
 struct liste_point *p0,*p1;

   if(no1<0 && no2<0){intersecte(cir0.nbp-2,cir1.nbp-2);return;}
   else if(no1<0){intersecte(cir0.nbp-2,no2);return;}
   else if(no2<0){intersecte(no1,cir1.nbp-2);return;}
printf(" verif_inter no1,no2,debut_inter %d %d %ld \n",no1,no2,debut_inter);

   if(debut_inter==NULL )return;
   li=debut_inter;
   while(li)
        {if(li->noa1==no1 && li->noa2==no2)
             {/* on supprime les inters ajoutees sur les 2 circuits */
		p0=NULL; p1=NULL;
                p0=test_sup_inter1(lcir0,no1,no2);  
                if(p0)
		  {p1=test_sup_inter1(lcir1,no2,no1);
                   if(p1){sup_inter1(lcir0,p0); sup_inter1(lcir1,p1);}
		  }
 	        break;
	     }
         li=li->suc;
	}
}
 /*--------------------------------------------------------------*/
struct liste_point *test_sup_inter1(lcir,no1,no2)
struct liste_point *lcir;
int no1,no2;
{ struct liste_point *lc,*lcpred,*lcird;
  no2=-(no2+1);
   printf("\n supprime inter no1= %d no2 = %d ",no1,no2);
  lcird=lcir;  lc=lcir;  lcpred=NULL;
  while(lc!=NULL&&lc->noinit!=no1){lcpred=lc;  lc=lc->suc;}
printf("\n lc = %ld noinit = %d suc = %ld no2 = %d \n",lc,lc->noinit,lc->suc,no2);
  do  {lcpred=lc;  lc=lc->suc;}
  while(lc!=NULL&&lc->noinit!=no2&&lc->noinit<0);
  if(lc&&lc->noinit<0) return(lc);
  /* liste_lcir_ecran(lcird); */
  return(NULL);
}

/*--------------------------------------------------------------*/
int sup_inter1(lcir,lcs)
struct liste_point *lcir,*lcs;
{ struct liste_point *lc,*lcpred,*lcird;
    lcird=lcir;  lc=lcir;  lcpred=NULL;
  while(lc!=lcs){lcpred=lc;  lc=lc->suc;}
       lcpred->suc=lc->suc; libere_liste_point(lc,2);	
   liste_lcir_ecran(lcird); 
}
/*--------------------------------------------------------------*/
int lit_intersecte1(no1,no2)
int no1,no2;
{struct inters *li;
   if(debut_inter==NULL)return(0);
   li=debut_inter;
   while(li)
        {if(li->noa1==no1 && li->noa2==no2)return(1);
         li=li->suc;
        }
   return(0);
}
/*--------------------------------------------------------------*/
int liste_poly(poly1)
struct spol *poly1;
{int i;
        printf("\nPOLY 1  nb pts = %d\n",poly1->nbp);
    for(i=0;i<poly1->nbp;i++)
        printf("   Statut Pt %2d :  %c  %c  %5.2lf %5.2lf\n",i+1,poly1->statut_point[i],poly1->statut_arete[i],poly1->x[i],poly1->y[i]);

}
