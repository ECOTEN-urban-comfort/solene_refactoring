/*
cc -c pers_util.c -lm
*/

/* DECOUPE_FACE_PLAN ET UTILITAIRES DE PERSPECTIVE */

/*----------------------------------------------------------*/
/*        a utiliser avec geomutile.c solutile.c            */
/*----------------------------------------------------------*/
/* MODIFIER par Groleau pour pers 1992 */

#include<solene.h>
#include<malloc.h>


struct modelisation_face *alloue_face();

/*____________________________________________________________________*/
/* declaration pers() */
double tgvis,covis,sinvis;

/*_________________________________________________*/
/* DECOUPE_FACE_PLAN                               */
/*_________________________________________________*/

/*----------------------------------------------------------*/
/*							    */
/*       decoupe d'une face  (ff) par 		            */
/*    un plan pn[0]*x+pn[1]*y+pn[2]*z+pn[3]=0               */
/*                                                          */
/*    coupe=1 on sait que la coupe existe                   */
/*    morceau=-1 chercher le DESSOUS seul                   */
/*   	     = 1 chercher le DESSUS  seul                   */
/*   	     = 0 chercher le DESSOUS et DESSUS              */
/*----------------------------------------------------------*/

int decoupe_face_plan(pn,ff,projete,fdessus,fdessous,coupe,morceau)
struct modelisation_face *ff,*fdessus,*fdessous;
double *pn;
int projete,coupe,morceau;
{double vect[3],vectt[3],xp,yp,zp,pt[3],pnt[3];
 struct modelisation_face *fact,*face1,*face2;
 int ind,i,face1_dessus;
 struct contour *pcont;

/*
printf("IMPRIME debut DECOUPE\n"); 
  liste_face(ff,projete);
  printf(" par plan %lf %lf %lf %lf\n", pn[0],pn[1],pn[2],pn[3]);
  printf(" coupe = %d \n",coupe);
*/
  if(projete)pcont=ff->debut_projete; else pcont=ff->debut_dessin;
  if(pcont==NULL)return(0);

 /* test si la face coupe le plan */
  if(coupe==0)
    { ind=test_face_plan(pn,ff,projete);
      if(ind==1){copie_face_projete(ff,projete,fdessus,1);
                 restore_par_aff(); return;
                }
      else if(ind==-1){copie_face_projete(ff,projete,fdessous,1);
                       restore_par_aff();return;
                      }
      else if(ind==2) { printf("\n le plan et la face sont paralleles\n");
                        return;
                      }
    }

 /* calcul la droite d'inter des 2 plans : 1 vecteur (vect) et 1 pt (xp,yp,zp) */
   ind=inter_plan_plan(ff->vnorm,pn,vect,&xp,&yp,&zp);
/*
printf(" droite d'inter vec = %lf %lf %lf \n",vect[0],vect[1],vect[2]);
printf("                pt  = %lf %lf %lf\n", xp,yp,zp);
*/
 /* transforme la face,la droite d'inter et la normale du plan de coupe */
 /* pour projection suivant normale a face a decouper */
   sauve_par_aff();
   obs.x=ff->vnorm[0]; obs.y=ff->vnorm[1]; obs.z=ff->vnorm[2];
   obs.xo=0; obs.yo=0; obs.zo=0; 
   tranfo(); 
   fact=alloue_face(1,105);

   fact->nofac_fichier=ff->nofac_fichier; 
   tran_face(ff,projete,fact,1);

   tranp(ff->vnorm[0],ff->vnorm[1],ff->vnorm[2],&(fact->vnorm[0]),&(fact->vnorm[1]),&(fact->vnorm[2]));

 /* fenetre pour normalisation  =fen face et normalisation */
   for(i=0;i<6;i++) fen_aff[i]=fact->fen[i];
   cal_fen_aff();
   normalise_face(fact,1);


/*
printf("IMPRIME apres Norm\n"); 
printf(" liste de la face normalisee \n");
liste_face(fact,1);
*/
   tranp(vect[0],vect[1],vect[2],&vectt[0],&vectt[1],&vectt[2]);
   tranp(xp,yp,zp,&pt[0],&pt[1],&pt[2]);
   normalise_point(pt[0],pt[1],pt[2],&pt[0],&pt[1],&pt[2]); 

 /* coupe la fenetre de projection par la droite d'inter */
   face1=alloue_face(1,108);
   face2=alloue_face(1,109);



   coupe_fen(vectt,pt,face1,face2);


/*
printf("IMPRIME face1\n"); liste_face(face1,1);
printf("IMPRIME face2\n"); liste_face(face2,1);
*/
 /* determine face dessus et dessous */
  tranp(pn[0],pn[1],pn[2],&pnt[0],&pnt[1],&pnt[2]);
  face1_dessus=test_face1_dessus(face1,face2,pnt);
 /* determine morceau face dessus ou/et dessous */


   if(face1_dessus)
      { if(morceau==-1||morceau==0) face1_inter_face2(fact,1,face2,1,fdessous,1);
        if(morceau==1||morceau==0) face1_inter_face2(fact,1,face1,1,fdessus,1);
      }
   else 
      { if(morceau==-1||morceau==0) face1_inter_face2(fact,1,face1,1,fdessous,1);
        if(morceau==1||morceau==0) face1_inter_face2(fact,1,face2,1,fdessus,1);
      }


   if(morceau==-1 || morceau==0) 
      { denormalise_face(fdessous,1); tran_face_inverse(fdessous,1);
        for(i=0;i<4;i++)fdessous->vnorm[i]=ff->vnorm[i];
        place_eq_plan_au_cir(fdessous,1);
        fdessous->nofac_fichier=ff->nofac_fichier;
      }
   if(morceau==1  || morceau==0)
      { denormalise_face(fdessus,1); tran_face_inverse(fdessus,1);
        for(i=0;i<4;i++)fdessus->vnorm[i]=ff->vnorm[i];
        place_eq_plan_au_cir(fdessus,1);
        fdessus->nofac_fichier=ff->nofac_fichier;
      }


   desalloue_face(fact);
   desalloue_face(face1);  desalloue_face(face2);
   restore_par_aff();

}

/*-----------------------------------------------------------*/
/*       test si une face coupe un plan infini pn            */
/* le dessus du plan est determine par le sens de la normale */
/*       ind = -1  la face est en dessous                    */ 
/*           =  0  la face coupe le plan                     */
/*           =  1  la face est  en dessus                    */
/*           =  2  la face est dans le plan                  */
/*  on regarde la face suivant la normale au plan de coupe   */
/*  le plan de coupe se trouve a une altitude z calculee pour*/
/*  le point x=1 et y=1 - pour savoir si la face est decoupee*/
/*  on compare les z des points de la face transformee au z  */
/*  du plan						     */
/*-----------------------------------------------------------*/

int test_face_plan(pn,face,projete)
struct modelisation_face *face;
double *pn;
int projete;
{ struct modelisation_face *fact;
  int ind,statut;
  struct circuit *pcir;
  struct contour *pc;
  double pnt[3],xp,yp,zp,val;
  int i;

   /* calcul dela transformation */
   sauve_par_aff();
   obs.x=pn[0]; obs.y=pn[1]; obs.z=pn[2];
   obs.xo=0; obs.yo=0; obs.zo=0;
   tranfo();

   /* calcul du z d'un point du plan  tranforme (= z du plan) */
   /* on prend le point (1,1,zp) */
   xp=1.;  yp=1.;  
   if(pn[2])zp=-(pn[3]+pn[1]+pn[0])/pn[2];
   else if(pn[1]){zp=0.; yp=-(pn[0]*xp+pn[3])/pn[1];}
   else {yp=0.; zp=0.; xp=-pn[3]/pn[0];}
  
   tranp(xp,yp,zp,&pnt[0],&pnt[1],&pnt[2]);

/* test sur les contours si plan decoupe face */
    fact=alloue_face(1,345);
    tran_face(face,projete,fact,1); 
    pc=fact->debut_projete;
    val=-999.999;
    while(pc)
	{pcir=pc->debut_support;
	 while(pcir)
	  {for(i=0;i<pcir->nbp;i++)
		{if(fabs(pcir->z[i]-pnt[2])>.001)
		   {if(val==-999.999)val=pcir->z[i]-pnt[2];
		    else if(val*(pcir->z[i]-pnt[2])<0)
                            { desalloue_face(fact);
                              restore_par_aff();
                              return(0);
                            }
                   }
		}
	   pcir=pcir->suc;
	  }
	 pc=pc->suc;
	}
    desalloue_face(fact);
    restore_par_aff();
    if(val<0)return(-1); else return(1);

}

/*-----------------------------------------------------------*/
/*  test si une fenetre englobante coupe un plan infini pn   */
/* le dessus du plan est determine par le sens de la normale */
/*       ind = -1  la fenetre est en dessous                 */ 
/*           =  0  la fenetre coupe le plan                  */
/*           =  1  la fenetre est  en dessus                 */
/*           =  2  la fenetre est dans le plan               */
/* PAS UTILISER POUR LE MOMENT                               */
/*-----------------------------------------------------------*/
int test_fenetre(pnt,fen)
double *fen,*pnt;
{double x[8],y[8],z[8];
 int i,j,k,id;
     id=0;
     for(j=0;j<4;j=j+3)
      {for(i=1;i<5;i=i+3)
        {for(k=2;k<6;k=k+3)
           { tranp(fen[j],fen[i],fen[k],x+id,y+id,z+id);
   	    z[id]=z[id]-pnt[2]; 
	    id++;
           }
        }
      }
    
  /* test les z de la fenetre par rapport au z du plan */
  /* cherche le premier z non nul */
   for(i=0;i<8;i++){if(z[i])break;}
   if(i==8)return(2);
   for(j=i+1;j<8;j++)
	{if(z[i]*z[j]<0) return(0);
	}
   if(z[i]<0)return(-1); else return(1);
}

/*-----------------------------------------------------------*/
int coupe_fen(vec,pt,face1,face2)
double  *vec,*pt;
struct modelisation_face *face1,*face2;
{double a,b,x1,y1;
 int i,j,k;
 struct circuit *pcir,*pciri;
 struct contour *pc;


     pcir=alloue_circuit(998);
     pcir->nbp=7;
     alloue_point_circuit(pcir,998);

 if(vec[0]==0)
   {  pcir->x[0]=-.1;  pcir->y[0]=-.1; pcir->z[0]=0.;
      pcir->x[1]=pt[0];  pcir->y[1]=-.1; pcir->z[1]=1.;
      pcir->x[2]=1.1;  pcir->y[2]=-.1; pcir->z[2]=0.;
      pcir->x[3]=1.1;  pcir->y[3]=1.1; pcir->z[3]=0.;
      pcir->x[4]=pt[0];  pcir->y[4]=1.1; pcir->z[4]=1.;
      pcir->x[5]=-.1;  pcir->y[5]=1.1; pcir->z[5]=0.;
      pcir->x[6]=-.1;  pcir->y[6]=-.1; pcir->z[6]=0.;
   }
 else
   {
     /* equation de la droite */
     a=vec[1]/vec[0];
     b=pt[1]-(vec[1]/vec[0])*pt[0];
     pcir->x[0]=-.1;  pcir->y[0]=-.1; pcir->z[0]=0;
     i=1;
     y1=-.1; x1=(y1-b)/a;
     if(x1<=1.1 && x1>-.1)
	{pcir->x[i]=x1;  pcir->y[i]=y1; pcir->z[i]=1; i++;}
     pcir->x[i]=1.1;  pcir->y[i]=-.1; pcir->z[i]=0; i++;

     x1=1.1; y1=x1*a+b;
     if(y1<=1.1 && y1>-.1)
	{pcir->x[i]=x1;  pcir->y[i]=y1; pcir->z[i]=1; i++;}
     pcir->x[i]=1.1;  pcir->y[i]=1.1; pcir->z[i]=0; i++;

     y1=1.1; x1=(y1-b)/a;
     if(x1<=1.1 && x1>-.1)
	{pcir->x[i]=x1;  pcir->y[i]=y1; pcir->z[i]=1;i++; }
     pcir->x[i]=-.1;  pcir->y[i]=1.1; pcir->z[i]=0; i++;

     x1=-.1; y1=x1*a+b;
     if(y1<=1.1 && y1>-.1)
	{pcir->x[i]=x1;  pcir->y[i]=y1; pcir->z[i]=1;i++; }
     pcir->x[i]=-.1;  pcir->y[i]=-.1; pcir->z[i]=0; 
    }
/*
for(i=0;i<7;i++)printf("pcir %lf %lf %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
*/
  /* constitue face 1 avec les pts inter comme pts 1 et 2 */ 
      pc=alloue_contour(234);
      pciri=alloue_circuit(234);
      pciri->nbp=7;
      pciri->vnorm[0]=0; pciri->vnorm[1]=0.;
      pciri->vnorm[2]=1.; pciri->vnorm[3]=0.;
      alloue_point_circuit(pciri,998);
      i=0;
      while(pcir->z[i]==0){ i++; if(i>6) 
                                   { printf("PB0 > 7 PTS\n");
                                     exit(0);
                                   }
                          }
/*printf(" i = %d \n",i);*/
      pciri->x[0]=pcir->x[i];  pciri->y[0]=pcir->y[i]; pciri->z[0]=0;
      i++; while(pcir->z[i]==0){ i++; if(i>7) 
                                   { printf("PB1 > 7 PTS\n");
                                     exit(0);
                                   }
                                }
      pciri->x[1]=pcir->x[i];  pciri->y[1]=pcir->y[i]; pciri->z[1]=0;
      k=2; i++;
      while(pcir->z[i]==0)
	{ pciri->x[k]=pcir->x[i];  pciri->y[k]=pcir->y[i]; pciri->z[k]=0;
          i++;i=i%(pcir->nbp-1);k++;
          if(i>7) 
              { printf("PB2 > 7 PTS\n");
                exit(0);
              }

        }
      pciri->x[k]=pciri->x[0];  pciri->y[k]=pciri->y[0]; pciri->z[k]=0;
      pciri->nbp=k+1;
      supt(pciri);
      if(pciri->nbp<4)
         { desalloue_contour(pc,120);
           desalloue_circuit(pciri,211);         
         }
      else
         { fenetre_circuit(pciri);
           pc->debut_support=pciri;
           face1->debut_projete=pc;
           face1->vnorm[0]=0; face1->vnorm[1]=0.;
           face1->vnorm[2]=1.; face1->vnorm[3]=0.;
           fen_face(face1,1);
         }

  /* constitue face 2 avec pt inter 1 en 1 et pt inter 2 en dernier pt */ 
      pc=alloue_contour(234);
      pciri=alloue_circuit(234);
      pciri->nbp=7;  
      pciri->vnorm[0]=0; pciri->vnorm[1]=0.;
      pciri->vnorm[2]=1.; pciri->vnorm[3]=0.;
      alloue_point_circuit(pciri,998);
      i=0;
      while(pcir->z[i]==0){ i++; if(i>7) 
                                   { printf("PB3 > 7 PTS\n");
                                     exit(0);
                                   }
                          }
      pciri->x[0]=pcir->x[i];  pciri->y[0]=pcir->y[i]; pciri->z[0]=0;
      i++; k=1;
      while(pcir->z[i]==0)
        { pciri->x[k]=pcir->x[i];  pciri->y[k]=pcir->y[i]; pciri->z[k]=0; k++;
          i++;
        }
      pciri->x[k]=pcir->x[i]; pciri->y[k]=pcir->y[i]; pciri->z[k]=0; k++;
      pciri->x[k]=pciri->x[0]; pciri->y[k]=pciri->y[0]; pciri->z[k]=0;
      pciri->nbp=k+1;
      supt(pciri);
      if(pciri->nbp<4)
         { desalloue_contour(pc,120);
           desalloue_circuit(pciri,212);         
         }
      else
         { fenetre_circuit(pciri);
           pc->debut_support=pciri;
           face2->debut_projete=pc;
           face2->vnorm[0]=0; face2->vnorm[1]=0.;
           face2->vnorm[2]=1.; face2->vnorm[3]=0.;
           fen_face(face2,1);
         }
  desalloue_circuit(pcir,214);
}
/*-----------------------------------------------------------------------------*/
int test_face1_dessus(face1,face2,vect)
struct modelisation_face *face1,*face2;
double *vect;
{    /* test si face1 dessus; 1 oui */
  struct contour *pc;
  struct circuit *pcir;
int i;
/*
printf("VECTEUR\n");
printf("%lf %lf %lf\n",vect[0],vect[1],vect[2]);
printf("FENETRE face1\n");
pc=face1->debut_projete;
pcir=pc->debut_support;
for(i=0;i<pcir->nbp;i++)printf("%lf %lf %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
printf("FENETRE face2\n");
pc=face2->debut_projete;
pcir=pc->debut_support;
for(i=0;i<pcir->nbp;i++)printf("%lf %lf %lf\n",pcir->x[i],pcir->y[i],pcir->z[i]);
printf("Coordonnees prises pour calcul face1 dessus\n");
*/
  pc=face1->debut_projete; /* calcul sur pt 1 et 3 */
  if(pc)
    { pcir=pc->debut_support;
      if(((pcir->x[2]-pcir->x[0])*vect[0]+(pcir->y[2]-pcir->y[0])*vect[1])>0)return(1);
      else return(0);
    }
  else 
    { pc=face2->debut_projete; /* calcul sur pt 1 et 2 */
      pcir=pc->debut_support;
      if(((pcir->x[1]-pcir->x[0])*vect[0]+(pcir->y[1]-pcir->y[0])*vect[1])>0)return(0);
      else return(1);
    }
}
/*-----------------------------------------------------------------------------*/
int face_coupe(fdessus,fdessous,pn,nbff)
struct modelisation_face *fdessus,*fdessous;
double *pn;
int *nbff;
{double val,*px,*py,*pz,prec;
 int i,j,k,kk,jj,nbp,nbf;
 struct circuit *pcir;
 struct contour *pc;
 nbf=*nbff;
 prec=.0001;
  px=alloue_double(nbf*2,100);
  py=alloue_double(nbf*2,101);
  pz=alloue_double(nbf*2,102);
  nbp=0;
  for(i=0;i<2*nbf;i++){px[i]=-999.999;py[i]=-999.999;pz[i]=-999.999;}
 /* recense sur chaque face les couple de points appartenant au plan de coupe */
  for(i=0;i<nbf;i++)
    {pc=(fdessus+i)->debut_projete;
     while(pc)
	{pcir=pc->debut_support;
	 while(pcir)
	  {kk=0;
           for(j=0;j<(pcir->nbp)-1;j++)
            {val=(pcir->x[j])*pn[0]+(pcir->y[j])*pn[1]+(pcir->z[j])*pn[2]+pn[3];
             if(fabs(val)<prec)
		{k=2*i;
                 if(kk>1)test_point_milieu(&px[k],&py[k],&pz[k],pcir->x[j],pcir->y[j],pcir->z[j]);
                 else {px[k+kk]=pcir->x[j]; py[k+kk]=pcir->y[j];  pz[k+kk]=pcir->z[j]; kk++;}
		}
            }
	   if(kk==1)printf("\n UN SEUL POINT SUR LA FACE %d \n",i);
	   else if(kk==2)nbp++;
	   pcir=pcir->suc; 
	  }
         pc=pc->suc;
	}
    }
 /* constitue la face */
    if(nbp==0)
       {printf("\n le volume n'a pas ete coupe par le plan \n");
	desalloue_double(px); desalloue_double(py);  desalloue_double(pz);
	return(0);
       }
    pcir=alloue_circuit(34); 
     pcir->nbp=nbp+1; 
     alloue_point_circuit(pcir);
     i=0;
     while(px[2*i]==-999.999)i++; 

      pcir->x[0]=px[2*i]; pcir->y[0]=py[2*i]; pcir->z[0]=pz[2*i];
      pcir->x[1]=px[2*i+1]; pcir->y[1]=py[2*i+1]; pcir->z[1]=pz[2*i+1];
     jj=1;
   while(1)
    {for(j=i+1;j<nbf;j++)
      {k=2*j;
       if(px[k]!=-999.999)
	{if(fabs(pcir->x[jj]-px[k])<prec&&fabs(pcir->y[jj]-py[k])<prec&&fabs(pcir->z[jj]-pz[k])<prec)
           {jj++;pcir->x[jj]=px[k+1]; pcir->y[jj]=py[k+1]; pcir->z[jj]=pz[k+1];
	    px[k]=-999.999; py[k]=-999.999; pz[k]=-999.999;
	    px[k+1]=-999.999; py[k+1]=-999.999; pz[k+1]=-999.999;
            break;
           }
         else if(fabs(pcir->x[jj]-px[k+1])<prec&&fabs(pcir->y[jj]-py[k+1])<prec&&fabs(pcir->z[jj]-pz[k+1])<prec)
            {jj++;pcir->x[jj]=px[k]; pcir->y[jj]=py[k]; pcir->z[jj]=pz[k];
	     px[k]=-999.999; py[k]=-999.999; pz[k]=-999.999;
	     px[k+1]=-999.999; py[k+1]=-999.999; pz[k+1]=-999.999;
             break;
            }
	}
      }
      if(fabs(pcir->x[jj]-pcir->x[0])<prec&&fabs(pcir->y[jj]-pcir->y[0])<prec&&fabs(pcir->z[jj]-pcir->z[0])<prec)
          break;
    }
  desalloue_double(px); desalloue_double(py); desalloue_double(pz);
  for(i=0;i<3;i++)pcir->vnorm[i]=pn[i];
  fenetre_circuit(pcir);

  for(i=0;i<6;i++)(fdessus+nbf)->fen[i]=pcir->fen[i];
  for(i=0;i<3;i++)(fdessus+nbf)->vnorm[i]=pn[i];

  (fdessus+nbf)->debut_projete=alloue_contour(346);
  (fdessus+nbf)->debut_projete->debut_support=pcir;
  
   calcul_d_du_plan(fdessus+nbf,1);
   place_eq_plan_au_cir(fdessus+nbf,1);
   

  copie_face_projete(fdessus+nbf,1,fdessous+nbf,1);
  nbf++;
  *nbff=nbf;

 }

/*_________________________________________________*/
/* UTILITAIRES DE PERSPECTIVE */
/*_________________________________________________*/

/*-----------------------------------------------------------*/
int visible_pers(face,projete)
struct modelisation_face *face;
int projete;
{ double scal;
  struct contour *pcont;
  struct circuit *pcir;
         /* retourne 1 si VISIBLE */
  if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
  if(pcont)
    { pcir=pcont->debut_support;  
      scal=face->vnorm[0]*(pcir->x[0]-obs.xo);
      scal+=face->vnorm[1]*(pcir->y[0]-obs.yo);
      scal+=face->vnorm[2]*(pcir->z[0]-obs.zo);
/*printf("scal %lf\n",scal);*/
      if(scal<0) return(1); else return(0);
    }
  else return(0);
}

/*-----------------------------------------------------------*/
int face_dans_vision(face,projete)
struct modelisation_face *face;
int projete;
{ 
 struct modelisation_face *fdessus,*fdessous;
 double pn[4];
 int resul;


 if((test_tout_derriere(face,projete))==0)
   { met_face_null(face,projete);
     return(0);
   }

 pn[3]=0;

 resul=test_coupe_pyramide(face,projete,1);
 if(resul==1) 
   { /* decoupe plan 1 */
     /*printf(" decoupe par plan 1\n");*/


     fdessus=alloue_face(1,890);
     fdessous=alloue_face(1,891);
     pn[0]=covis; pn[1]=0; pn[2]=sinvis;




     decoupe_face_plan(pn,face,projete,fdessus,fdessous,1,-1);

/*
liste_pointeur_contour();
*/
/*printf("conserve face dessous\n");*/
     copie_face_projete(fdessous,1,face,projete);


/*
printf("Apres decoupe PLAN 1\n");
liste_face(face,projete);
*/

     desalloue_face(fdessus);
     desalloue_face(fdessous);

   }
 else if(resul==-1)
   { met_face_null(face,projete);
     return(0);
   }

 resul=test_coupe_pyramide(face,projete,2);
 if(resul==1) 
   { /* decoupe plan 2 */
  /*   printf(" decoupe par plan 2\n"); */
     fdessus=alloue_face(1,892);
     fdessous=alloue_face(1,893);
     pn[0]=-covis; pn[1]=0; pn[2]=sinvis;
/*
printf("avant decoupe\n");
liste_face(face,projete);
*/

     decoupe_face_plan(pn,face,projete,fdessus,fdessous,1,-1);
/*
printf(" decoupe plan 2\n"); liste_pointeur_contour();
*/


/*printf("conserve face dessous\n");*/
     copie_face_projete(fdessous,1,face,projete);


/*
printf("Apres decoupe PLAN 2\n");
liste_face(face,projete);
*/

     desalloue_face(fdessus);
     desalloue_face(fdessous);
   }
 else if(resul==-1)
   { met_face_null(face,projete);
     return(0);
   }

 resul=test_coupe_pyramide(face,projete,3);
 if(resul==1) 
   { /* decoupe plan 3 */
 /*    printf(" decoupe par plan 3\n"); */
     fdessus=alloue_face(1,894);
     fdessous=alloue_face(1,895);
     pn[0]=0; pn[1]=covis; pn[2]=sinvis;
/*
printf("avant decoupe\n");
liste_face(face,projete);
*/

     decoupe_face_plan(pn,face,projete,fdessus,fdessous,1,-1);
/*
printf(" decoupe plan 3\n"); liste_pointeur_contour();
*/
/*printf("conserve face dessous\n");*/
     copie_face_projete(fdessous,1,face,projete); 

/*
printf("apres decoupe PLAN 3\n");
liste_face(face,projete);
*/

     desalloue_face(fdessus);
     desalloue_face(fdessous);
   }
 else if(resul==-1)
   { met_face_null(face,projete);
     return(0);
   }

 resul=test_coupe_pyramide(face,projete,4);
 if(resul==1) 
   { /* decoupe plan 4 */
/*     printf(" decoupe par plan 4\n"); */
     fdessus=alloue_face(1,896);
     fdessous=alloue_face(1,897);
     pn[0]=0; pn[1]=-covis; pn[2]=sinvis;
/*
printf("avant decoupe\n");
liste_face(face,projete);
*/ 

     decoupe_face_plan(pn,face,projete,fdessus,fdessous,1,-1);
/*
printf(" decoupe plan 4\n"); liste_pointeur_contour();
*/

/*printf("conserve face dessous\n");*/
     copie_face_projete(fdessous,1,face,projete);


/*
printf("apres decoupe PLAN 4\n");
liste_face(face,projete);
*/

     desalloue_face(fdessus);
     desalloue_face(fdessous);
   }
 else if(resul==-1)
   { met_face_null(face,projete);
     return(0);
   }
}

/*-----------------------------------------------------------*/
int met_face_null(face,projete)
struct modelisation_face *face;
int projete;
{
      if(projete)
        { desalloue_chaine_contour(face->debut_projete,1030); 
          face->debut_projete=NULL;
        }
      else
        { desalloue_chaine_contour(face->debut_dessin,1031);
          face->debut_dessin=NULL;
        }
}

/*-----------------------------------------------------------*/
int test_tout_derriere(face,projete)
struct modelisation_face *face;
int projete;
{
  int i;
  struct contour *pcont;
  struct circuit *pcir;

            /* le test z<0 est fait sur les seuls contours support */
  if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
  while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
           { if(pcir->z[i]<0) return(1);             
           }
         pcont=pcont->suc; 
       } 
  return(0);
}


/*-----------------------------------------------------------*/
int test_coupe_pyramide(face,projete,plan)
struct modelisation_face *face;
int projete,plan;
{
  int i,oui,tout_derriere,tout_devant;
  struct contour *pcont;
  struct circuit *pcir;
            /* le test est fait sur les seuls contours support */
/*  printf("test PLAN %d\n",plan); */
  tout_derriere=1; tout_devant=1;
  if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
  while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
           { oui=test_coupe_plan(pcir->x[i],pcir->y[i],pcir->z[i],plan);
             if(oui==0) tout_derriere=0; else tout_devant=0;
             if(tout_devant==0 && tout_derriere==0) return(1);
           }
         pcont=pcont->suc; 
       } 
  if(tout_derriere==1) return(-1);
  else return(0);
}

/*-----------------------------------------------------------*/
int test_coupe_plan(x,y,z,plan)
double x,y,z;
int plan;
{
 if(plan==1)
   { if(z<0) { if(fabs(x/z)>tgvis && x>0) return(1); 
               else return(0);
             }
     else if(z==0) { if(x>0) return(1);
                     else return(0);
                   }
     else { if(fabs(x/z)>=tgvis && x<0) return(0);
            else return(1);
          }
   }
 else if(plan==2)
   { if(z<0 ) { if(fabs(x/z)>tgvis && x<0) return(1);
                else return(0);
              }
     else if(z==0) { if(x<0) return(1);
                     else return(0);
                   }
     else return(1);
   }
 else if(plan==3)
   { if(z<0) { if(fabs(y/z)>tgvis && y>0) return(1); 
               else return(0);
             }
     else if(z>=0) { if(y>0) return(1);
                     else return(0);
                   }
     else return(0);
   }
 else
   { if(z<0) { if(fabs(y/z)>tgvis && y<0) return(1); 
               else return(0);
             }
     else if(z>=0) return(1);
     else return(0);
   }
}
/*-----------------------------------------------------------*/
int pers_face_inverse(face,projete)
struct modelisation_face *face;
int projete;
{
 struct contour *pcont;
 struct circuit *pcir;
 int i;

   if(projete)pcont=face->debut_projete; else pcont=face->debut_dessin;
   while(pcont)	   
       { pcir=pcont->debut_support; 
         for(i=0;i<pcir->nbp;i++)
            { /* calcul z avec equation du plan  et x,y en pers */
              calcul_z_pers(face,pcir->x[i],pcir->y[i],&(pcir->z[i]));
              pcir->x[i]=-pcir->x[i]*pcir->z[i];
              pcir->y[i]=-pcir->y[i]*pcir->z[i];
            }
	 pcir=pcont->debut_interieur;
         while(pcir)
	    { for(i=0;i<pcir->nbp;i++) 
                { /* calcul z avec equation du plan  et x,y en pers */
                  calcul_z_pers(face,pcir->x[i],pcir->y[i],&(pcir->z[i]));
                  pcir->x[i]=-pcir->x[i]*pcir->z[i];
                  pcir->y[i]=-pcir->y[i]*pcir->z[i];
                }
              fenetre_circuit(pcir);
	      pcir=pcir->suc;
	    }
         pcont=pcont->suc; 
       } 
}


/*-----------------------------------------------------------*/
int calcul_z_pers(face,xp,yp,z)
struct modelisation_face *face;
double xp,yp,*z;
{
  /* calcul z du pt en perspective xp,yp */
 *z=(face->vnorm[3])/((face->vnorm[0])*xp+(face->vnorm[1])*yp-face->vnorm[2]);
}
