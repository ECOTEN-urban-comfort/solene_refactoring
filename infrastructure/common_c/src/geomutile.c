

/*** UTILITAIRES GEOMETRIQUES ******/

/* 
cc  -c  geomutile.c  -lm 
*/

#include<solene.h>
double coef_discol=.001;

/*_______________________________________________________________________*/
/*                        INTERSECTION                                   */
/*           SURFACE-ANGLES -PERIMETRES-DISTANCE-NORMALES		         */
/*      SUPPRESSION ET COLLAGE DE POINTS DANS UN PLOLYGONE               */
/*        TEST SUR UN POLY ET POINT/POLY POINT/DROITE POINT/FACE         */
/*                 CENTRE DE GRAVITE                                     */
/*_______________________________________________________________________*/

/*_______________________________________________________________________*/
/*                        INTERSECTION                                   */
/*_______________________________________________________________________*/
/*-----------------------------------------------------*/
/* la droite est definie par 2 points P1 et P2         */
/* le plan par sa normale et son coefficient d         */
/*						       */
/*   retour = -1 la droite p1p2 est  parallele au plan */
/*          =  0 pas d'intersection                    */
/*          =  1 une intersection = p1                 */
/*          =  2 une intersection = p2                 */
/*          =  3 une intersection entre p1 et p2       */
/*          =  4 la droite p1p2 est dans le plan       */              
/*-----------------------------------------------------*/

int inter_droite_plan(pn,x1,y1,z1,x2,y2,z2,xint,yint,zint)
double *pn,x1,y1,z1,x2,y2,z2,*xint,*yint,*zint;
{double a,b,xi,yi,zi;

  a=pn[0]*x1+pn[1]*y1+pn[2]*z1+pn[3];
  b=pn[0]*x2+pn[1]*y2+pn[2]*z2+pn[3];
  b=b-a; 
/* b est le produit scalaire de la normale au plan et de la droite */

  if(fabs(b)>DISCOL*coef_discol)
    {
/* l'egalite suivante est obtenue en ecrivant que le point d'intersection */
/* Pi appartient au plan et a la droite 				  */
    a=-a/b; 

     xi=a*(x2-x1)+x1;   yi=a*(y2-y1)+y1;    zi=a*(z2-z1)+z1;
/* produit scalaire des vecteurs p1pi et p2pi */
/* si b >0 les deux points sont situes du meme cote du plan */
     b=(x1-xi)*(x2-xi)+(y1-yi)*(y2-yi)+(z1-zi)*(z2-zi);
     if(fabs(b)<DISCOL*coef_discol)
            {if(fabs(xi-x1)<DISCOL*coef_discol && fabs(yi-y1)<DISCOL*coef_discol && fabs(zi-z1)<DISCOL*coef_discol)
                {*xint=x1; *yint=y1; *zint=z1; return(1);}
             else
		{*xint=x2; *yint=y2; *zint=z2; return(2);}
	     }
     else if(b>0)return(0);
     else {*xint=xi;  *yint=yi;  *zint=zi;
           return(3);
          }
    }
  else
    {if(fabs(a)<DISCOL*coef_discol)return(4); /* la droite est dans le plan */
     else return(-1);   /* la droite est parallele au plan */
    }

}

/*____________________________________________________________________*/
/* intersection de 2 droites  d1 et d2 definies par les coordonnees   */
/* deleurs extremites	[0] et [1]				      */
/*  retour = -1 paralleles ou colineaires			      */
/*         =  0 pas d'intersections				      */
/*         =  1 une inter de coordonnees xint,yint		      */
/*  t1=0 inter sur extremite [0] de d1				      */
/*  t1=1 inter sur extremite [1] de d1				      */
/*  t2=0 inter sur extremite [0] de d2				      */
/*  t2=1 inter sur extremite [1] de d2				      */
/*____________________________________________________________________*/

int inter(x1,y1,x2,y2,xint,yint,t1,t2)
double *x1,*y1,*x2,*y2,*xint,*yint;
double *t1,*t2;
{ double xba,yba,xcd,ycd,xca,yca,d,d1,d2,prec;
   prec=DISCOL*coef_discol;
  /* prec=DISCOL;*/
   xba=x1[1]-x1[0]; yba=y1[1]-y1[0];
   xcd=x2[0]-x2[1]; ycd=y2[0]-y2[1];
   d1=dmax(fabs(xba),fabs(yba));
   d2=dmax(fabs(xcd),fabs(ycd));

   d=xba*ycd-xcd*yba;  
   /* test si paralleles ou colineaires */
    if(fabs(d)<0.000000001){return(-1);} /* 10-4 */
 
    xca=x2[0]-x1[0];   yca=y2[0]-y1[0];
    *t1=(xca*ycd-xcd*yca)/d;       *t2=(xba*yca-yba*xca)/d;
    if(fabs(1-*t1)<prec/d1) *t1=1;       if(fabs(*t1)<prec/d1) *t1=0;
    if(fabs(1-*t2)<prec/d2) *t2=1;       if(fabs(*t2)<prec/d2) *t2=0;
    xca=x2[1]-x1[0];   yca=y2[1]-y1[0];

    if(xca==0 && yca==0){*t1=0; *t2=1;}
    else
	{xca=x2[0]-x1[1]; yca=y2[0]-y1[1];
	 if(xca==0 && yca==0){*t1=1; *t2=0;}
	 else
	      {xca=x2[1]-x1[1];  yca=y2[1]-y1[1];
	       if(xca==0 && yca==0){*t1=1; *t2=1;}
	      }
	}
     if(*t1<0||*t1>1||*t2<0||*t2>1)return(0);
     *xint=x1[0]+(*t1)*xba; *yint=y1[0]+(*t1)*yba;
     return(1);
}
/*-----------------------------------------------------------*/
/*            intersection de 2 plan p1 et p2		     */
/*     retour ind = -1 disjoints (paralleles)                */
/*                   0 intersection (vecteur + point)        */
/*                   1 p1=p2				     */
/*-----------------------------------------------------------*/
int inter_plan_plan (p1,p2,vect,xp,yp,zp)
double *p1,*p2,*vect,*xp,*yp,*zp;
{double delta;

 /* la droite d'intersection des 2 plans est donnee par un vecteur vect  */
 /* et un point P (xp,yp,zp) - le vecteur vect est egal au produit       */
 /* vectoriel des normales aux plans - le point P est calculee en fixant */
 /* une coordonnee et en calculant les 2 autres en resolvant le systeme  */
 /* compose par les 2 equations de plan				         */

  vect[0]=p1[1]*p2[2]-p1[2]*p2[1];
  vect[1]=p1[2]*p2[0]-p1[0]*p2[2];
  vect[2]=p1[0]*p2[1]-p1[1]*p2[0];
  if(vect[0]==0 && vect[1]==0 && vect[2]==0)
	{if(p1[3]/p2[3]==p1[0]/p2[0])return(1); 
                /* les plans sont identiques */
	 else return(-1);
	       /* les normales sont identiques - les 2 plans sont paralleles */
	}
  delta=vect[2];
  if(delta==0)
    {delta=vect[1];
     if(delta==0)
	{ /* la droite d'inter est dirigee suivant x */
	  delta=vect[0];
	 *xp=0; *yp=(p2[3]*p1[2]-p2[2]*p1[3])/delta;
         *zp=(p2[1]*p1[3]-p2[3]*p1[1])/delta;
	}
     else {/* la droite d'inter est // au plan xoy */
	   *yp=0; *xp=(p2[2]*p1[3]-p2[3]*p1[2])/delta;
           *zp=(p2[3]*p1[0]-p2[0]*p1[3])/delta;
	  }
    }		
  else { /* la droite d'inter est un plan quelconque */
	 *zp=0; *xp=(p2[3]*p1[1]-p2[1]*p1[3])/delta;
         *yp=(p2[0]*p1[3]-p2[3]*p1[0])/delta;
        }
 return(0);
}
/*_______________________________________________________________________*/
/*           SURFACE-ANGLES -PERIMETRES-DISTANCES			 */
/*_______________________________________________________________________*/

int calcule_normale_face(face)
struct modelisation_face *face;
{
 struct contour *pcont ;
 struct circuit *pcir ;
 double nor[4], scal ;
 int n0,n1,n2 ;
 int i ,j,k ;
 double xx[3],yy[3] ,zz[3] ;
 double surf, surfmax ;

pcont = face->debut_projete ;
pcir = pcont->debut_support ; 

// calcule la surface max de 3 pts consécutifs ou non du circuit
surfmax = 0 ;
for ( i=0 ; i<= pcir->nbp-4 ; i++)
{ for ( j=i+1 ; j<= pcir->nbp-3 ; j++)
  { for ( k=j+1 ;k<= pcir->nbp-2 ; k++)
     { // calcule surf pour le triangle constitué des points d'indice i,j,k
        // en projection sur XOY, XOZ et YOZ
        xx[0] = pcir->x[i] ; yy[0] = pcir->y[i] ; zz [0] = pcir->z[i] ;
        xx[1] = pcir->x[j] ; yy[1] = pcir->y[j] ; zz [1] = pcir->z[j] ;
        xx[2] = pcir->x[k] ; yy[2] = pcir->y[k] ; zz [2] = pcir->z[k] ;

        surf = fabs(surface(3,xx,yy)) ;
        if (surf > surfmax) 
         { surfmax   = surf ;
            n0 = i ; n1 = j ; n2 =k ;
         }
        surf = fabs(surface(3,xx,zz)) ;
        if (surf > surfmax) 
         { surfmax   = surf ;
            n0 = i ; n1 = j ; n2 =k ;
         }
        surf = fabs(surface(3,yy,zz)) ;
        if (surf > surfmax) 
         { surfmax   = surf ;
            n0 = i ; n1 = j ; n2 =k ;
         }
     }
  }
}

// calcule la normale sur les 3 pts donnant la plus gde surface
xx[0] = pcir->x[n0] ; yy[0] = pcir->y[n0] ; zz [0] = pcir->z[n0] ;
xx[1] = pcir->x[n1] ; yy[1] = pcir->y[n1] ; zz [1] = pcir->z[n1] ;
xx[2] = pcir->x[n2] ; yy[2] = pcir->y[n2] ; zz [2] = pcir->z[n2] ;

normale_avec_3pts(xx,yy,zz,nor) ;

// ne s'occupe pas du sens de la normale

face->vnorm[0] = nor[0] ;
face->vnorm[1] = nor[1] ;
face->vnorm[2] = nor[2] ;

}

/*____________________________________________________________________*/
/*____________________________________________________________________*/
double surface(nbp,x,y)
int nbp;
double *x,*y;
{int i;
 double surf;
 surf=0;
 for(i=0;i<nbp-1;i++) surf+=x[i+1]*y[i]-x[i]*y[i+1];
 surf+=x[0]*y[nbp-1]-x[nbp-1]*y[0];

 return(surf/2);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int airpol(nbp,x,y)
int nbp;
double *x,*y;
{double s,p;
 int i;
  for(i=0;i<nbp-3;i++)
     {s=surface(3,&x[i],&y[i]);
      p=perimetre_poly(3,&x[i],&y[i]);
      if(s<p*DISCOL*coef_discol/4.)
       {/* supprime le point i+1 */
	/*printf("\n surface trop petite : point %d supprime ");*/
       }
     }
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

double surface_point(p1,p2,p3)
struct point *p1,*p2,*p3;
{
 double surf;
  surf=(p2->x)*(p1->y)-(p2->y)*(p1->x); 
  surf+=(p3->x)*(p2->y)-(p3->y)*(p2->x);
  surf+=(p1->x)*(p3->y)-(p1->y)*(p3->x); 
   return(fabs(surf/2));
}
/*----------------------------------------------------------------------*/
/*   calcul du cosinus de l'angle forme par deux vecteurs consecutifs  */
/*----------------------------------------------------------------------*/
double cos_angle_vec(x1,y1,x2,y2)
double x1,y1,x2,y2;
{double co;
   co=(x1*x2+y1*y2)/sqrt((x1*x1+y1*y1)*(x2*x2+y2*y2));
   return(co);
}

/*----------------------------------------------------------------------*/
/*   calcul du cosinus de l'angle forme par deux vecteurs en 3D         */
/*----------------------------------------------------------------------*/
double cos_angle_vec_3D(x1,y1,z1,x2,y2,z2)
double x1,y1,z1,x2,y2,z2;
{double co;
   co=(x1*x2+y1*y2+z1*z2)/sqrt((x1*x1+y1*y1+z1*z1)*(x2*x2+y2*y2+z2*z2));
   return(co);
}
/*----------------------------------------------------------------------*/
/*  calcul du perimetre d'un polygone ferme (si ouvert on le ferme)     */
/*----------------------------------------------------------------------*/
double perimetre_poly(nbp,x,y)
int nbp;
double *x,*y;
{int i;
 double per;
 per=0;
 for(i=0;i<nbp-1;i++) per+=sqrt((x[i+1]-x[i])*(x[i+1]-x[i])+(y[i+1]-y[i])*(y[i+1]-y[i]));
 per+=sqrt((x[0]-x[nbp-1])*(x[0]-x[nbp-1])+(y[0]-y[nbp-1])*(y[0]-y[nbp-1]));
 return(per);
}

/*-----------------------------------------------------------------------------*/

double surface_face(pfac)
struct modelisation_face *pfac;
{double surf;
            sauve_par_aff();
            obs.x=pfac->vnorm[0];  obs.y=pfac->vnorm[1];  
            obs.z=pfac->vnorm[2];
            tranfo();  
            tran_face(pfac,1,pfac,0); 
	    surf=cal_surf_face(pfac,0);
            restore_par_aff();
            return(surf); 
         
}

/*-----------------------------------------------------------------------------*/
double cal_surf_face(pfac,projete)
struct modelisation_face *pfac;
int projete;
{double surf;
 struct contour *pc;
 struct circuit *pcir;
  if(projete==1)pc=pfac->debut_projete;
  else pc=pfac->debut_dessin;
  surf=0;
  while(pc)
	{pcir=pc->debut_support;
	 surf+=fabs(surface(pcir->nbp,pcir->x,pcir->y));
	 pcir=pc->debut_interieur;
	 while(pcir)
	    {surf-=fabs(surface(pcir->nbp,pcir->x,pcir->y));
	     pcir=pcir->suc;
	    }
         pc=pc->suc;
	}
  return(surf);
}

/*____________________________________________________________________*/
/*      ANGLE_MIN = valeur du cosinus de l'angle minimum  	      */
/*____________________________________________________________________*/

int angle_pol(nbp,x,y)
int nbp;
double *x,*y;
{double s;
 int i;
  for(i=0;i<nbp-3;i++)
    {s=cos_angle_vec(x[i]-x[i+1],y[i]-y[i+1],x[i+2]-x[i+1],y[i+2]-y[i+1]);
     if(s<ANGLE_MIN)
       {/* supprime le point i+1 */
	/*sprintf(imp,"\n angle trop petit : point %d supprime ");imprime();*/
       }
    }
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int imax(a,b)
int a,b;
{
if(a>b)return(a); else return(b);
}

/*____________________________________________________________________*/
/*____________________________________________________________________*/

double dmax(a,b)
double a,b;
{
if(a>b)return(a); else return(b);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

double dmin(a,b)
double a,b;
{
if(a<b)return(a); else return(b);
}
/*-----------------------------------------------------------*/
/*__________________________________________________________*/
int normale_avec_3pts(x,y,z,nor)
double *x,*y,*z,*nor;
{double x1,y1,z1,x2,y2,z2;
  x1=x[1]-x[0]; y1=y[1]-y[0]; z1=z[1]-z[0];
  x2=x[2]-x[1]; y2=y[2]-y[1]; z2=z[2]-z[1];
  nor[0]=y1*z2-y2*z1;  nor[1]=-x1*z2+x2*z1;  nor[2]=x1*y2-x2*y1;
  x1=sqrt(nor[0]*nor[0]+nor[1]*nor[1]+nor[2]*nor[2]);
  if(x1)
  {nor[0]=nor[0]/x1; nor[1]=nor[1]/x1; nor[2]=nor[2]/x1;}
}

/*____________________________________________________________________*/
/* calcul de la normale sur les 3 premiers points d'un circuit        */
/*____________________________________________________________________*/
int normale(pcir,nor)
double *nor;
struct circuit *pcir;
{double x1,y1,z1,x2,y2,z2;
 int i;
 i=0;
while(1)
 {
  x1=pcir->x[i+1]-pcir->x[i]; y1=pcir->y[i+1]-pcir->y[i]; z1=pcir->z[i+1]-pcir->z[i];
  x2=pcir->x[i+2]-pcir->x[i+1]; y2=pcir->y[i+2]-pcir->y[i+1]; z2=pcir->z[i+2]-pcir->z[i+1];
  nor[0]=y1*z2-y2*z1;  nor[1]=-x1*z2+x2*z1;  nor[2]=x1*y2-x2*y1;
  x1=sqrt(nor[0]*nor[0]+nor[1]*nor[1]+nor[2]*nor[2]);
  if(x1)break;
  i++;
  if(i==pcir->nbp-1){x1=1;break;}
 }
  nor[0]=nor[0]/x1; nor[1]=nor[1]/x1; nor[2]=nor[2]/x1;
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

double test_distance(x0,y0,x1,y1,x2,y2)
double x0,y0,x1,y1,x2,y2;
{double a,b,c,d;

    a=fabs(x1-x0); b=fabs(x2-x0); c=fabs(y1-y0); d=fabs(y2-y0);
    if(fabs(a-b)<DISDE/1000 && fabs(c-d)<DISDE/1000)return(-999.999);

  if(fabs(a-b)>fabs(c-d))return(a-b);
  return(c-d);

}
/*____________________________________________________________________*/
/*      SUPPRESSION ET COLLAGE DE POINTS DANS UN PLOLYGONE            */
/*____________________________________________________________________*/

int supt_point(p)
struct point *p;
{int nbp;
struct point *pcour,*pt;
  /*sprintf(imp,"\n suppression de points "); imprime();*/

    nbp=1;  pcour=p; 
    p=p->suc;
    while(p)
    	{if(fabs((pcour->x)-(p->x))>DISCOL*coef_discol||fabs((pcour->y)-(p->y))>DISCOL*coef_discol)
             {pcour->suc=p;nbp++;pcour=p;p=p->suc;}
	 else {/*sprintf(imp,"\n point supprime ");imprime();*/
               if(p->suc==NULL)*pcour=*p;
                pt=p;p=p->suc;desalloue_point(pt,3);
              }
        }
   /*sprintf(imp,"\n retour nbp = %d ",nbp);imprime();*/
    return(nbp);
}


/*____________________________________________________________________*/
/*____________________________________________________________________*/
int sup_surf_circuit(pcir)
struct circuit *pcir;
{int i,nbp;
 struct point *pt,*ptsuc,*debut;
  if(pcir->nbp<4)return(0);
  pt=alloue_point();  if(pt==NULL)pb_allocation(21);
  pt->suc=NULL; debut=pt;
  pt->x=pcir->x[0]; pt->y=pcir->y[0]; pt->z=pcir->z[0];
  for(i=1;i<pcir->nbp;i++)
	{pt->suc=alloue_point(); if(pt->suc==NULL)pb_allocation(22);
         pt=pt->suc;  pt->suc=NULL;
	 pt->x=pcir->x[i]; pt->y=pcir->y[i]; pt->z=pcir->z[i];
	}
  nbp=pcir->nbp;
  sup_surf(&nbp,&debut); 
  if(nbp>3)
	{desalloue_point_circuit(pcir,6);
         pcir->nbp=nbp;
         alloue_point_circuit(pcir,10);
	 pt=debut; i=0;
	 while(pt)
	   {ptsuc=pt->suc; pcir->x[i]=pt->x; pcir->y[i]=pt->y; pcir->z[i]=pt->z; 
            desalloue_point(pt,4);
	    pt=ptsuc;  i++;
	   }
	}
   else {pt=debut; while(pt){ptsuc=pt->suc;desalloue_point(pt,5);pt=ptsuc;}}
   pcir->nbp=nbp;
}
/*____________________________________________________________________*/
/*______________________________________________________________________*/
int sup_surf(nbp,debut) /*suppression de surface ds une liste de points */
struct point **debut;
int *nbp;
{struct point *p,*pt;
 int fin,ind,test;
 if(*nbp<=3)return(0);
fin=1;
while(fin)
  {fin=0;test=0;
   p=*debut;
   while(p && p->suc->suc)
    {if(surface_point(p,p->suc,p->suc->suc)<DISCOL*coef_discol)
          {ind=test_point_interieur(*debut,p,p->suc,p->suc->suc);
	   if(*nbp<=5||ind==0)
             { fin=1;
               pt=p->suc; p->suc=p->suc->suc; desalloue_point(pt,6);
               (*nbp)--;
               if(*nbp==3)return(0);
	     }
            else
             {test=1;
              p=p->suc; 
             }
           }
       else 
           {if(*nbp==4)return(0);
            p=p->suc; 
           }
    }
   if(*nbp>3)
     /* test du dernier triangle */
     {if(surface_point(p,*debut,(*debut)->suc)<DISCOL*coef_discol)
       {ind=test_point_interieur(*debut,p,*debut,(*debut)->suc);
	if(*nbp<=5||ind==0)
          { fin=1;
           *(p->suc)=*((*debut)->suc);p->suc->suc=NULL;p=*debut;
	   *debut=(*debut)->suc;desalloue_point(p,7);(*nbp)--; 
           if(*nbp==3)return(0);
	  }
        else  test=1;
       }
      else if(*nbp==4)return(0);
     }
  }
 if(test==1)
    { printf(" FACE en TRAITEMENT %d\n",traite_face1);
      printf("\n suppression de surface impossible - pt interieur au triangle\n");
            pt=*debut;
            while(pt)
		{printf(" %lf %lf %lf \n",pt->x,pt->y,pt->z);
		 pt=pt->suc;
		}
   }
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int test_point_interieur(debut,p1,p2,p3) /* point interieur  return 1 */
struct point *debut,*p1,*p2,*p3;         /*                  return 0 */
{struct circuit *pcir;
 struct point *pt;
 int idedan,isur;
 /* constitution du triangle */
  pcir=alloue_circuit(30);
  pcir->nbp=4;
  alloue_point_circuit(pcir,31);
  pcir->x[0]=p1->x; pcir->y[0]=p1->y; pcir->z[0]=p1->z;
  pcir->x[1]=p2->x; pcir->y[1]=p2->y; pcir->z[1]=p2->z;
  pcir->x[2]=p3->x; pcir->y[2]=p3->y; pcir->z[2]=p3->z;
  pcir->x[3]=p1->x; pcir->y[3]=p1->y; pcir->z[3]=p1->z;
  pt=debut; idedan=0;
  while(pt)
  {if(pt!=p1 && pt!=p2 && pt!=p3)
     {inpoly(&idedan,&isur,pt->x,pt->y,pcir,DISCOL);
      if(idedan==1){/*printf("\n sup impossible \n");
		    liste_circuit_ecran(pcir);
	            printf("\n pt interieur %lf %lf \n",pt->x,pt->y);*/
		    break;
		   }
     }
   pt=pt->suc;
  }
 desalloue_circuit(pcir,32);
 if(idedan==0)return(0);
 
 return(1);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int supt(pcir)
struct circuit *pcir;
{int nbps,i;
 struct circuit *pciri,*pcirsuc;
    pciri=alloue_circuit(26);
    if(pciri==NULL)pb_allocation(20);
    copie_cir(pcir,pciri); 
      pcirsuc=pcir->suc;
    nbps=0;  
  for(i=1;i<pciri->nbp;i++)
        {if(fabs(pciri->x[nbps]-pciri->x[i])>DISCOL*coef_discol||fabs(pciri->y[nbps]-pciri->y[i])>DISCOL*coef_discol)
         {pciri->x[nbps+1]=pciri->x[i]; pciri->y[nbps+1]=pciri->y[i]; 
          pciri->z[nbps+1]=pciri->z[i]; nbps+=1;
         }
      /*   else printf(" point no  %d supprime \n",i);*/
        }
     pciri->x[nbps]=pciri->x[0]; pciri->y[nbps]=pciri->y[0]; pciri->z[nbps]=pciri->z[0]; 
     pciri->nbp=nbps+1;
  if(pcir->nbp!=pciri->nbp)
      { copie_cir(pciri,pcir);
        pcir->suc=pcirsuc;
      }
 desalloue_circuit(pciri,10);
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int  colpt(pcir0,pcir1)
struct circuit *pcir0,*pcir1;
{ int i,j;
 /*sprintf(imp,"\n colle les points ");imprime();*/
  for(i=0;i<pcir0->nbp;i++)
	{for(j=0;j<pcir1->nbp;j++)
		{if(fabs(pcir0->x[i]-pcir1->x[j])<DISCOL*coef_discol&&fabs(pcir0->y[i]-pcir1->y[j])<DISCOL*coef_discol)
			{ /*printf("\n points colles");*/
                          pcir0->x[i]=pcir1->x[j];pcir0->y[i]=pcir1->y[j];
                        }
		}
	}
}
/*____________________________________________________________________*/
/*    TEST SUR UN POLY ET POINT/POLY POINT/DROITE POINT/FACE          */
/*        TEST SENS D'UN POLY ET POINT/POLY POINT/DROITE              */
/*____________________________________________________________________*/

/*--------------------------------------------------------------------*/
/* POINT DANS FACE                                                    */
/*--------------------------------------------------------------------*/
int point_dans_face(xp,yp,face,projete)
double xp,yp;
struct modelisation_face *face;
int projete;
{             /* si pt interieur a face : return(1) */
              /* cherche pour chaque contour de la face */
              /* avec verification si trou */
 int idedan,isur;
 struct contour *pcont;
 struct circuit *pcir;

   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
   if((point_dans_fenetre(xp,yp,face->fen))==0) return(0);
   while(pcont)	   
       { pcir=pcont->debut_support; 
                 /* test pt / fenetre circuit */
         if(point_dans_fenetre(xp,yp,pcir->fen))
            { inpoly(&idedan,&isur,xp,yp,pcir);
              if(isur==1) return(0);
              if(idedan)
                 {         /* test si point ds trou ; si oui idedan=0 */
	           pcir=pcont->debut_interieur;
                   while(pcir)    
	              {    /* test pt / fenetre circuit */
                        if(point_dans_fenetre(xp,yp,pcir->fen))
                          { inpoly(&idedan,&isur,xp,yp,pcir);
                            if(idedan || isur) return(0);
                          }
	                pcir=pcir->suc;
	              }
                   return(1);
                 }
            }
         pcont=pcont->suc; 
       } 
  return(0);
}

/*-----------------------------------------------------------------------------*/
int point_dans_fenetre(xp,yp,fen)
double xp,yp,*fen;
{       
      /* si pt interieur a fen circuit : return(1) */

   if(xp < fen[0] || xp > fen[3]) return(0);
   if(yp < fen[1] || yp > fen[4]) return(0);
   return(1);
}

/*____________________________________________________________________*/
/*  pour idedan et isur 0=.false.  1=.true.			      */
/*____________________________________________________________________*/
int inpoly(idedan,isur,x,y,pcir)
int *idedan,*isur;
double x,y;
struct circuit *pcir;
{int i,ia,id;
 double xinter,dx,dy;
 double epsilone;

   epsilone = 0.01;
   //printf("epsilone %f\n",epsilone);
   *idedan=0; *isur=0;
  for(i=0;i<(pcir->nbp)-1;i++)
	{if(pcir->y[i]!=pcir->y[i+1])
		{if(pcir->y[i]>pcir->y[i+1]){id=i; ia=i+1;}
		 else {id=i+1; ia=i;}
		 /* arete orientee vers le bas */
		  if(pcir->y[ia]==y){if(pcir->x[ia]==x){*isur=1;*idedan=0;return(0);}}

		   else if(pcir->y[ia]<y)
			  {if(pcir->y[id]>=y && (pcir->x[id]<=x||pcir->x[ia]<=x))
				{if(pcir->x[id]==pcir->x[ia])xinter=pcir->x[id];
				  else{dx=pcir->x[ia]-pcir->x[id]; dy=pcir->y[ia]-pcir->y[id];
					xinter=pcir->x[id]+(y-pcir->y[id])*dx/dy;
				       }
				 if(xinter==x){*isur=1;*idedan=0;return(0);}

				 else if(xinter<x)*idedan=!(*idedan);
				}
			  }
			
		}
          else if(pcir->y[i]==y){if((x-pcir->x[i])*(x-pcir->x[i+1])<0){*isur=1;*idedan=0;return(0);}}
	}
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/
int sens_face(face,proj,nb)
struct modelisation_face *face;
int proj,nb;
{
 int i;
 struct contour *pcont; 
 struct circuit *pcir;
   for(i=0;i<nb;i++)
      {  if(proj)pcont=(face+i)->debut_projete;
         else pcont=(face+i)->debut_dessin;
         while(pcont)
            { pcir=pcont->debut_support;
              sensz(pcir); 
              pcir=pcont->debut_interieur;
              while(pcir)
                 { sensz(pcir); 
                   pcir=pcir->suc;
                 }
              pcont=pcont->suc;
            }
      }
}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int sensz(pcir)
struct circuit *pcir;
{
 /* calcul de la surface du circuit */
  if(surface(pcir->nbp,pcir->x,pcir->y)>0)invsens(pcir);

}
/*____________________________________________________________________*/
/*____________________________________________________________________*/

int invsens(pcir)
struct circuit *pcir;
{int i,j;
 double s;
 /*printf("\n inversion du sens ");*/
 for(i=1;i<(pcir->nbp)/2;i++)
    {j=pcir->nbp-i-1;
     s=pcir->x[i];pcir->x[i]=pcir->x[j]; pcir->x[j]=s;
     s=pcir->y[i];pcir->y[i]=pcir->y[j]; pcir->y[j]=s;
     s=pcir->z[i];pcir->z[i]=pcir->z[j]; pcir->z[j]=s;
    }
  }

/*____________________________________________________________________*/
/*____________________________________________________________________*/

int tstin(pcir,xp,yp,nos)
struct circuit *pcir;
double xp,yp;
int nos;
{int i,j,k,kk;
 double t1,t2;
 double x,y,xs,ys;
   /*sprintf(imp,"\n tstin %lf %lf %d",xp,yp,nos);imprime();*/
  xs=pcir->x[nos]; ys=pcir->y[nos];
  pcir->x[nos]=xp; pcir->y[nos]=yp;
  if(nos==0){pcir->x[pcir->nbp-1]=xp; pcir->y[pcir->nbp-1]=yp;}
  fen_arete(pcir,nos,fen);
/* si c'est un triangle on verifie le sens du circuit */
 if(pcir->nbp<5)
       {if(surface(pcir->nbp,pcir->x,pcir->y)>0)
              {if(nos==0){pcir->x[pcir->nbp-1]=xs; pcir->y[pcir->nbp-1]=ys;} return(1);}
       }
 else
 {/*if(nos==pcir->nbp-2)k=1; else */k=0;
  i=0;
  while(i<pcir->nbp-4)
    {j=(nos+k+2)%(pcir->nbp-1);
     fen_arete(pcir,j,fen+1);
     if(recoup(fen,fen+1)!=-1)
     {
     kk=inter(&(pcir->x[nos]),&(pcir->y[nos]),&(pcir->x[j]),&(pcir->y[j]),&x,&y,&t1,&t2);
     if(t1==1 || t1==0 || t2==1 || t2==0)kk=0;
     if(kk==1)
	{/* printf("\n nos = %d j = %d nbp = %d ",nos,j,pcir->nbp); */
         /* printf(imp,"\n t1 = %lf t2= %lf ",t1,t2); */
	 pcir->x[nos]=xs; pcir->y[nos]=ys;
	 if(nos==0){pcir->x[pcir->nbp-1]=xs; pcir->y[pcir->nbp-1]=ys;}
	 return(1);
	}
     }
     i++;k++;
    }
  }
  return(0);
}

/*--------------------------------------------------------------------------*/
int test_point_milieu(x,y,z,xx,yy,zz)
double *x,*y,*z,xx,yy,zz;
{ if(fabs(x[0]-x[1])>.01)
	{if((x[0]-x[1])*(x[0]-xx)<0){x[0]=xx; y[0]=yy; z[0]=zz;}
         else if((x[1]-x[0])*(x[1]-xx)<0){x[1]=xx; y[1]=yy; z[1]=zz;}
	}
  else if(fabs(y[0]-y[1])>.01)
	{if((y[0]-y[1])*(y[0]-yy)<0){x[0]=xx; y[0]=yy; z[0]=zz;}
         else if((y[1]-y[0])*(y[1]-yy)<0){x[1]=xx; y[1]=yy; z[1]=zz;}
	}
  else if(fabs(z[0]-z[1])>.01)
	{if((z[0]-z[1])*(z[0]-zz)<0){x[0]=xx; y[0]=yy; z[0]=zz;}
         else if((z[1]-z[0])*(z[1]-zz)<0){x[1]=xx; y[1]=yy; z[1]=zz;}
	}
}

/*--------------------------------------------------------------------------*/
/*                 CENTRE DE GRAVITE                                        */
/*--------------------------------------------------------------------------*/
/*_________________________________________________________________*/
void centre_de_gravite(pcir, xg, yg, zg)
struct circuit *pcir;
double *xg, *yg, *zg;
{int k;

if (pcir->nbp <2)
	{printf("Erreur calcul centre de gravite\n");
	 exit(0);
	}

*xg=0.0; *yg=0.0; *zg=0.0;
for(k=0;k<pcir->nbp-1;k++) 
   { 
	*xg+=pcir->x[k];
    *yg+=pcir->y[k];
    *zg+=pcir->z[k];
   }
      
*xg=*xg/(pcir->nbp-1);
*yg=*yg/(pcir->nbp-1);
*zg=*zg/(pcir->nbp-1);

}
