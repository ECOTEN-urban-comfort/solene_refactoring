/*

cc  -c  traite.c  -lm 

D. GROLEAU //MODIF dec 2005 ; modifie appel à coplanaire (solutile) dans traite_moins et traite_moins_hel
			// pb de calcul de coplénarité (les contours sont projetés et normalisés
			// donc pour le MOMENT, on SHUNTE l'appel à coplanaire dans traite_moins
			// d'AILLEURS, EST-CE BIEN NECESSAIRE (on ne gagne pas grand chose)
			// si fait, alors le faire de manière interne à traite.c
*/

#include<solene.h>
#include<malloc.h>

int coplanaire();

//MODIF dec 2005
//#define EPSI_COPLAN 0.00001     // pour les test de coplanerite des faces
//MODIF dec 2005 FIN

/*-----------------------------------------------------------------------------*/
/* realise les operations booleennes sur un fichier de faces                   */
/*-----------------------------------------------------------------------------*/

int traite_moins(ff,nbff)
int nbff;
struct modelisation_face *ff;
{int i,j,k,idev,ider,kk,ind,ll;
int ic;
struct modelisation_face *ff1;

//MODIF dec 2005
double EPSI_COPLAN;
int facteur_D;

 EPSI_COPLAN= 0.00001;     // pour les test de coplanerite des faces
 facteur_D=1000;            // pour les test de meme plan
//MODIF dec 2005 FIN


  // les faces sont dans   projete 
 cir0.nbp=0; cir1.nbp=0;
 for(i=0;i<nbff;i++)
   {  
    traite_face1=ff[i].nofac_fichier;
    if(ff[i].debut_projete)
       {//printf("\n traite la face %d \n",ff[i].nofac_fichier);
	    for(j=0;j<i;j++)
         {
             traite_face2=ff[j].nofac_fichier;
             if(ff[j].debut_projete)
	           {/* fait le traitement si les faces ne sont pas coplanaires  */
				 // mais dans coplanaire ne recalcule pas le d du plan
//MODIF dec 2005
//		    	 if(!coplanaire(ff+i,1,ff+j,1,EPSI_COPLAN,0,facteur_D))
					 //attention au calcul de coplanaire, même D dans solutile
					 // le test est fait sur les valeurs en projection normalisée
					 // le test de coplanérité serait à revoir avec EPSI_COPLAN et facteur_D
					 // A REVOIR
//MODIF dec 2005 FIN
				  {
					//printf("   compare avec la face %d nofac = %d \n",ff[j].nofac_fichier,j);

				    /* compare les fenetres face-fface dans le plan (x ,y ) */
					if(recoup_circuit(ff[i].fen,ff[j].fen)!=-1)
					 {/* cherche qui devant-derriere sur contour d'origine */

					  ind=devder(i,j,ff);

					  /* printf(" \n devder return ind = %d \n",ind); */
					  if(ind==0)
						{/* c'est la face j qui est devant */
					     ider=i; idev=j;
						}
					  else if(ind==1)
						{/* c'est la face i qui est devant */
						 ider=j; idev=i;
 						}
					  else {/*printf("\n les faces sont disjointes ");*/}
                                 /* alloue un contour resultat pour etape 1+2 */
					  if(ind!=-1)
						{/* traite les faces */
							test_sens(ff+ider,0);
						 /*printf(" face1_moins_face2\n");*/
						  //printf(" face1-face2 %d-%d (devant face %d)\n",traite_face1,traite_face2,(ff+idev)->nofac_fichier);
							ind=face1_moins_face2(ff+ider,0,ff+idev,1,ff+ider,0); 

							test_sens(ff+ider,0);
						/*printf(" test_sens\n");*/
						}
   					 }
				  }
		      }
	     }
     }
    }
  desalloue_point_circuit_cir(&cir0,7);desalloue_point_circuit_cir(&cir1,8);
}
/*-----------------------------------------------------------------------------*/

int traite_moins_hel(f1,nbf1,f2,nbf2)
int nbf1,nbf2;
struct modelisation_face *f1,*f2;
{int i,j,k,idev,ider,kk,ind,ll;
  struct modelisation_face *fc;
			
//MODIF dec 2005
double EPSI_COPLAN;
int facteur_D;

 EPSI_COPLAN= 0.00001;     // pour les test de coplanerite des faces
 facteur_D=1000;            // pour les test de meme plan
//MODIF dec 2005 FIN

  // les faces sont dans   projete 
	fc=alloue_face(2,12);

 cir0.nbp=0; cir1.nbp=0;
 for(i=0;i<nbf2;i++)
   { 
    traite_face2=f2[i].nofac_fichier;

    if(f2[i].debut_projete)
       {//printf("\n traite la face %d \n",f2[i].nofac_fichier); 
		for(j=0;j<nbf1;j++)
         {
            /* compare les fenetres face-fface dans le plan (x ,y ) */
            traite_face1=f1[j].nofac_fichier;
            if(f1[j].debut_projete)
	           { //fait le traitement si les faces ne sont pas coplanaires 
			//MODIF dec 2005
				 //if(!coplanaire(f2+i,1,f1+j,1,EPSI_COPLAN,2)) // avant
				 //if(!coplanaire(f2+i,1,f1+j,1,EPSI_COPLAN,0,facteur_D)) // après
			//MODIF dec 2005 FIN

			      { //printf(" compare avec la face %d nofac = %d \n",f1[j].nofac_fichier,j);
					if(recoup_circuit(f1[j].fen,f2[i].fen)!=-1)
		              {/* cherche qui devant-derriere sur contour d'origine */
                       /*printf(" cherche qui devant-derriere sur contour d'origine \n");*/
	                    copie_face(f2+i,fc+1);
	                    copie_face(f1+j,fc);

                        ind=devder(0,1,fc); 
						/*printf(" devder ind = %d \n",ind);*/

						if(ind==0)
							{/* traite les faces */
							 test_sens(f1+j,0);

							 ind=face1_moins_face2(f1+j,0,f2+i,1,f1+j,0);  

							 test_sens(f1+j,0); 
							}
   					 }
	             }
	          }
	     }
       }
    }
  desalloue_fface(fc,2);
  desalloue_point_circuit_cir(&cir0,7);desalloue_point_circuit_cir(&cir1,8);
}

/*-----------------------------------------------------------------------------*/
/*             renvoi 1 si coplanaire ou si i est devant , 0 si j est devant ,-1 si disjoints   */
/*-----------------------------------------------------------------------------*/


int devder(i,j,ff)
int i,j;
struct modelisation_face *ff;
{int k,kk,idedan,isur,coupe;
 double t1,t2; 
 double x,y,zp,zpp,ecart;
 struct circuit *pcir,*pciri;
 struct contour *pcont,*pcont1;
   ecart=0;
    /* si les 2 z sont egaux, on effectue le moins ; non maintenant avec test coplanaire */
   /* compare les fenetres ff-ff */
   if(ff[i].fen[2]>=ff[j].fen[5])return(1);
   else if(ff[j].fen[2]>=ff[i].fen[5])return(0);
   /* les 2 fenetres  des 2 faces s'intersectent */
   coupe=0;
   pcont=ff[i].debut_projete;
   while(pcont)
    { pcir=pcont->debut_support;
      if(recoup_circuit(pcir->fen,ff[j].fen)!=-1)
       {
         pcont1=ff[j].debut_projete;
         while(pcont1)
          { pciri=pcont1->debut_support;
            if(recoup_circuit(pcir->fen,pciri->fen)!=-1)
              { coupe=1;
                /* test si un point de la ff i est interieur a la ff j */
	       for(k=0;k<pcir->nbp-1;k++)
	         {inpoly(&idedan,&isur,pcir->x[k],pcir->y[k],pciri);
	          if(idedan==1)
                    {zp=calcul_zplan(pciri,pcir->x[k],pcir->y[k]);
	             if(fabs((pcir->z[k])-zp)>fabs(ecart))ecart=zp-pcir->z[k];
                    }
     	         }
               if(fabs(ecart)>.1){if(ecart>0)return(0); else return(1);}

               /* test si un point de la ff j est interieur a la ff i */
               for(k=0;k<pciri->nbp-1;k++)
	        {inpoly(&idedan,&isur,pciri->x[k],pciri->y[k],pcir);
	         if(idedan==1)
                   {zp=calcul_zplan(pcir,pciri->x[k],pciri->y[k]);
                    if(fabs((pciri->z[k])-zp)>fabs(ecart))ecart=pciri->z[k]-zp;
                   }
     	        }
              if(fabs(ecart)>.1){if(ecart>0)return(0); else return(1);}

               /* test si un point commun */
               for(k=0;k<pcir->nbp-1;k++)
	        {for(kk=0;kk<pciri->nbp-1;kk++)
	          {if(fabs(pcir->x[k]-pciri->x[kk])<DISCOL && fabs(pcir->y[k]-pciri->y[kk])<DISCOL)
	  	     {if(fabs(pcir->z[k]-pciri->z[kk])>fabs(ecart))
                                       ecart=pciri->z[kk]-pcir->z[k];
	             }
	          }
                }
              if(fabs(ecart)>.1){if(ecart>0)return(0); else return(1);}

             }
	   pcont1=pcont1->suc;
	 }
       }
      pcont=pcont->suc;
    }

 if(coupe==0) return(-1); 
 pcont=ff[i].debut_projete;
   while(pcont)
    { if(recoup_circuit(pcir->fen,ff[j].fen)!=-1)
        { pcir=pcont->debut_support;
          pcont1=ff[j].debut_projete;
          while(pcont1)
            { pciri=pcont1->debut_support;
              if(recoup_circuit(pcir->fen,pciri->fen)!=-1)
                {
                  /* test sur les intersections */
                  for(k=0;k<pcir->nbp-1;k++)
                    {fen_arete(pcir,k,fen);
                     for(kk=0;kk<pciri->nbp-1;kk++)
	               {fen_arete(pciri,kk,fen+1);
	                /* test si les fenetres des 2 aretes se coupent */
	                if(recoup(fen,fen+1)!=-1)
	                  {/* cherche inter */ 
                           isur=inter(&(pcir->x[k]),&(pcir->y[k]),&(pciri->x[kk]),&(pciri->y[kk]),&x,&y,&t1,&t2);
                           if(isur==1&&((t1!=0&&t1!=1)||(t2!=0&&t2!=1)))
		             { /* calcul du z sur l'intersection */
		               zp=calcul_zplan(pcir,x,y);
		               zpp=calcul_zplan(pciri,x,y);
              	                if(fabs(zp-zpp)>fabs(ecart))ecart=zpp-zp;
	                     }
	                  }
	               }	
	             }
                   if(fabs(ecart)>DISCOL){if(ecart>0)return(0); else return(1);}
               }
            pcont1=pcont1->suc;
          }
        }
       pcont=pcont->suc;
     }
  return(-1);
}
/*-----------------------------------------------------------------------------*/
int traite_union_nouveau(ff,nbff,funion)
int nbff;
struct modelisation_face *ff,*funion;
{int i,j,k,idev,ider,kk,ind,ll;
                   /* UTILISER par masque_sol.c */
                   /* FAIRE mise a jour union.c */

                   /* resultat dans funion->PROJETE */
                   /* les faces a unir sont dans ff->DESSIN */

 for(i=0;i<nbff;i++)
   { /*printf("traite la face %d ",ff[i].nofac_fichier);*/
     if(ff[i].debut_dessin)
       { if(funion->debut_projete)
            { if(recoup_circuit(ff[i].fen,funion->fen)!=-1)
                 {   /* cherche union avec face i */
                   face1_union_face2(funion,1,ff+i,0,funion,0);
                   copie_face_projete(funion,0,funion,1);
                   desalloue_chaine_contour(funion->debut_dessin,1050);
                   funion->debut_dessin=NULL;
                 }
              else
                 {  /* rajoute la face i a l'union */
                   concatene_fface(ff+i,0,funion,1);
                 }
            }
         else       /* initialise funion a face i */
            { concatene_fface(ff+i,0,funion,1);
            }
       }
   }
}

/*-----------------------------------------------------------------------------*/

int traite_union(ff,nbff)
int nbff;
struct modelisation_face *ff;
{int i,j,k,idev,ider,kk,ind,ll;
 for(i=0;i<nbff;i++)
   {if(ff[i].debut_dessin)
       {/*printf("\r traite la face %d ",ff[i].nofac_fichier);*/
        for(j=0;j<i;j++)
         {if(ff[j].debut_dessin)
	       {/*printf(" compare avec la face %d nofac = %d \n",ff[j].nofac_fichier,j);*/
                if(recoup_circuit(ff[i].fen,ff[j].fen)!=-1)
	   	   {/* traite les faces */
		     ind=face1_union_face2(ff+j,0,ff+i,0,ff+i,0);
                     desalloue_chaine_contour((ff+j)->debut_projete,1051);
		     (ff+j)->debut_projete=NULL;
                     desalloue_chaine_contour((ff+j)->debut_dessin,1052);
		     (ff+j)->debut_dessin=NULL;
                   }
	        }
	   }
       }
    }
  desalloue_point_circuit_cir(&cir0,7);desalloue_point_circuit_cir(&cir1,8);
}
/*-----------------------------------------------------------------------------*/

int traite_inter(nodeb,nofin)
int nodeb,nofin;
{int i,j,k,idev,ider,kk,ind;
 for(kk=nodeb;kk<=nofin;kk++)
   {for(i=fface[kk].debut;i<fface[kk].fin;i++)
     {if(face[i].debut_projete)
       { k=face[i].nofac_fichier;
         /*printf("\r traite la face %d contour %d  nofac = %d",k,i-fface[k].debut+1,i);  */
        for(j=0;j<i;j++)
         {if(face[j].debut_dessin && (face[j].nofac_fichier!=face[i].nofac_fichier))
	   {printf("\n compare avec la face %d \n",j);
            /* compare les fenetres face-face dans le plan (x ,y ) */
	    if(recoup_circuit(face[i].fen,face[j].fen)!=-1)
		{ider=j; idev=i;
 		 /* traite les faces */
		  ind=face1_inter_face2(&face[ider],0,&face[idev],0,&face[ider],0);
                }
   	   }
         }
	}
      }
  }
  desalloue_point_circuit_cir(&cir0,7);desalloue_point_circuit_cir(&cir1,8);
}
