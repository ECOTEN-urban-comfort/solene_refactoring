
                             
                              /*    FACE_OP_FACE.C  */

// C. MARENNE et D. GROLEAU, MODIF mars 2006 dans etape3_union et dans etape2_union (voir MODIF mars 2006)
//					on considère dans etape3 et 2 qu'on gère seulement les 2 contours à unir
/*
cc -c face_op_face.c -lm
*/

#include<solene.h>

extern int option_calcul_z; /* option CALCUL du Z ds FACE-OP-FACE 1=oui */

struct circuit *copie_trou_circuits();
struct modelisation_face *alloue_face();
struct contour *copie_contour();
struct contour *union_chaine_contour();
struct contour *chaine_union_chaine();
struct spol *copy_cir_poly();
int test_epure;
/*-----------------------------------------------------------------------------------*/
/*                                    FACE MOINS FACE	face2-face1 	            
 */
/*-----------------------------------------------------------------------------------*/
int face1_moins_face2(face2,proj2,face1,proj1,face_resul,proj)

struct modelisation_face *face1,*face2,*face_resul;
int proj1,proj2,proj;
{struct contour *pcont1,*pcont2;
 struct contour *pcont,*pcdev,*pdeb,*pc,*pcontcour,*resul_debut,*resul_cour;
 struct contour *contour_resul,*contour_resul_cour,*pc_face_resul;
 int ind,id;

//printf(" face1_moins_face2 \n");

//printf(" option_calcul_z %d\n", option_calcul_z);

 //
		init_cir(&cir0);
		init_cir(&cir1);
           /* test si face1 est nulle */
           ind=0; id=0;
           if(proj1){if(face1->debut_projete==NULL)ind=1;}
           else{if(face1->debut_dessin==NULL)ind=1;}
	   if(ind==0)id=recoup_circuit(face1->fen,face2->fen);
             /* test les fenetres englobantes */
           if(ind==1||id==-1)
		{copie_face_projete(face2,proj2,face_resul,proj); return(-1);}
           if(proj1)pcont1=face1->debut_projete;
           else pcont1=face1->debut_dessin;
           if(proj2)pcont2=face2->debut_projete;
           else pcont2=face2->debut_dessin;
	   pc_face_resul=pcont2;
           pcdev=pcont1;


           while(pcdev)
              {pcont=pc_face_resul; contour_resul=NULL;pc_face_resul=NULL;
              while(pcont)
                     {resul_debut=NULL; resul_cour=NULL; 
                      ind=etape1_moins(&resul_debut,&resul_cour,pcont,pcdev);
                      if(ind==1)
				{/* pour le premier contour union du vide cree aux trous orig. */
				   id=etape4_moins(pcont,resul_debut);
 				   if(id!=1)
			              {/* pour les contours suivants  */
				       /* superpose les parties vues des supports suivants aux trous */
				       /* du support original */
		  		          pdeb=resul_debut->suc;
				          etape3_moins(pcont,&pdeb);
				          resul_debut->suc=pdeb;
				      }
				    else 
				      {ind=-1;
				       /* desalloue le contour resultat resul_debut */
                                        desalloue_chaine_contour(resul_debut,1019); resul_debut=NULL;
				      }
         		    	}
		      else if(ind==2||ind==0)
				{/* determine les parties du support situee derriere */
				 /* vues par les trou du contour situe devant   */
                                    id=etape2_moins(pcont,&resul_debut,&resul_cour,pcdev);
				 /* superpose les parties vues des supports aux trous */
				 /* du support original */
                                   if(id!=2) etape3_moins(pcont,&resul_debut);
				   else {desalloue_chaine_contour(resul_debut,1020);resul_debut=NULL;ind=-1;}
                                }
                    if(ind!=-1)
		       {/* insere le resultat   dans les contours de la face */
			if(resul_debut!=NULL) 
		            {pc=resul_debut;
			     if(contour_resul==NULL)contour_resul=pc;
			     else pcontcour->suc=pc;
			     while(pc->suc)pc=pc->suc;   pcontcour=pc;
			    }
		       }
		     else /* les 2 contours sont disjoints */
                       { /* copie du contour traite dans le resultat */
		         if(contour_resul==NULL)
			    {contour_resul=copie_contour(pcont,contour_resul);
			     pcontcour=contour_resul;
			    }
			 else {pcontcour->suc=copie_contour(pcont,pcontcour->suc);
		               pcontcour=pcontcour->suc;
			      }
           	       }
                       /* prend le contour suivant de la face */
		       pcont=pcont->suc;
                  }
             if(contour_resul!=NULL)
                  {if(pc_face_resul==NULL)pc_face_resul=contour_resul;
                   else contour_resul_cour->suc=contour_resul;
                   pcont=contour_resul;
		   while(pcont->suc)pcont=pcont->suc;
                   contour_resul_cour=pcont;
                  }
                pcdev=pcdev->suc;
	      }
  if(proj){if(face_resul->debut_projete)desalloue_chaine_contour(face_resul->debut_projete,1021);
	    face_resul->debut_projete=pc_face_resul;
	   }
   else   {if(face_resul->debut_dessin)desalloue_chaine_contour(face_resul->debut_dessin,1022);
	   face_resul->debut_dessin=pc_face_resul;
	  }
    fen_face(face_resul,proj);
    epure_face(face_resul,proj); 


	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);


}
/******************************************************************/
int epure_face(face,proj)
struct modelisation_face *face;
int proj;
{struct circuit *pcir;
 struct spol *poly;
 struct contour *pcont,*pcont_resul,*pcontpred,*pcontsuc,*pcont2,*pc,*pccour;
 int trouve,test;
 struct circuit *pcird,*pcirsuc;

    if(proj)pcont=face->debut_projete;
    else pcont=face->debut_dessin;
    pcontpred=NULL;
    while(pcont)
	{pcontsuc=pcont->suc;
         pcir=pcont->debut_support;
	 poly=copy_cir_poly(pcir);
         pcont_resul=alloue_contour(123);
         test_epure=0;
	 epure_polygone_face(face,poly,pcont_resul);
	 test=0;
	 if(test_epure==1)
              {/* test si un circuit est dans le mauvais sens = interieur */
               pcir=pcont_resul->debut_support;
	       if(pcir==NULL)test=2;
	       while(pcir)
		{if(surface(pcir->nbp,pcir->x,pcir->y)>0){test=1; break;}
	         pcir=pcir->suc;
                }
	      }
/*
printf(" epure_face() \n");
liste_face(face,proj);
printf(" test = %d test_epure = %d \n",test,test_epure); 
*/

          if(test==1 && test_epure==1)/*cas du circuit interieur*/
	      {pcir=pcont_resul->debut_support;
	       trouve=0;
               pcont2=alloue_contour(234);
	       while(pcir)
		{pcirsuc=pcir->suc;
		 if(trouve==1||surface(pcir->nbp,pcir->x,pcir->y)>0)
                          {if(surface(pcir->nbp,pcir->x,pcir->y)<0)
				  /*liste_struct_contour_ecran(pcont);*/
                           invsens(pcir);
		           pcird=pcont2->debut_interieur;
			   pcont2->debut_interieur=pcir;
			   pcir->suc=pcird; 
		           if(pcird!=NULL)pcird->suc=NULL;
		          }
		 else{trouve=1;
                      pcont2->debut_support=pcir;
		      pcir->suc=NULL;
		     }
		 pcir=pcirsuc;
                }
/*
printf("\n trouve = %d \n",trouve);
*/
             if(trouve==0){libere_contour(pcont2,5); 
			   desalloue_contour(pcont);
			   pcont2=NULL;
	                   if(pcontpred==NULL)
		  		 {if(proj)face->debut_projete=pcont2;
                    		  else face->debut_dessin=pcont2;
		   		 }
	         	   else pcontpred->suc=pcontsuc;
			  }

               /* affecte les trous de pcont2 a pcont */
	      else
                {pcont2->debut_interieur=pcont->debut_interieur;
                 desalloue_chaine_circuit(pcont->debut_support,8);
	         libere_contour(pcont,2);
	         if(pcontpred==NULL)
		   {if(proj)face->debut_projete=pcont2;
                    else face->debut_dessin=pcont2;
		   }
	         else pcontpred->suc=pcont2;
	         pcont2->suc=pcontsuc;
	         pcontpred=pcont2;
		}
               libere_contour(pcont_resul,3);
	     }
         else if(test_epure==1&&test==0)/*decomposition en 2 circuits independants*/
                                        /* le circuit decompose est exterieur */
	      {      pcir=pcont_resul->debut_support->suc;
		     pcont_resul->debut_support->suc=NULL;
		     pccour=pcont_resul;
		     while(pcir)
			{pcirsuc=pcir->suc;
			 pc=alloue_contour(23);
			 pccour->suc=pc;
		         pc->debut_support=pcir;
		         pccour=pc;
		         pcir->suc=NULL;
		         pcir=pcirsuc;
			}
	       test_interieur(pcont_resul,pcont);
                desalloue_chaine_circuit(pcont->debut_support,8);  
		libere_contour(pcont,4);
               if(pcontpred==NULL)
		 {if(proj)face->debut_projete=pcont_resul;
                  else face->debut_dessin=pcont_resul;
		 }
	       else pcontpred->suc=pcont_resul;
               pccour->suc=pcontsuc;
	       pcontpred=pcont_resul;
	      }
	 else if(test_epure==1&&test==2) /* le circuit decompose est vide */
	      {if(pcontpred==NULL)
		 {if(proj)face->debut_projete=pcontsuc;
                  else face->debut_dessin=pcontsuc;
		 }
		 else pcontpred->suc=pcontsuc;
	       desalloue_contour(pcont_resul,10);desalloue_contour(pcont,10);
	      }
         else {desalloue_contour(pcont_resul,10); pcontpred=pcont;}
	 pcont=pcontsuc; 
	}
}
/******************************************************************/
int test_interieur(pconte,pconti)
struct contour *pconte,*pconti;
{struct contour *pcont;
 struct circuit *pciri,*pcir,*pcirisuc;
 int idedan,isur,i,trouve;
  pciri=pconti->debut_interieur;
  while(pciri)
	{pcirisuc=pciri->suc; trouve=0;
 	 pcont=pconte;
	 while(pcont)
	  {pcir=pcont->debut_support;
	   isur=1; i=0;
	   while(isur&&i<pciri->nbp-1)
	     {inpoly(&idedan,&isur,pciri->x[i],pciri->y[i],pcir,DISCOL);
              if(idedan==1)
		{pciri->suc=pconte->debut_interieur;
		 pconte->debut_interieur=pciri;
	      	}
              trouve=1; isur=0;
	      i++;
	     }
	  if(trouve==1)break;
          pcont=pcont->suc;
	 }
        if(trouve==0){printf("\n epure_face interieur non affecte\n");}
        pciri=pcirisuc;
       }
}
 /******************************************************************/
int epure_polygone_face(face,poly,pcont)
struct modelisation_face *face;
struct spol *poly;
struct contour *pcont;
{ struct circuit *cir;
  struct spol *poly_dec;
  int i;
if(recherche_pt_commun_ds_poly(poly))
    {  test_epure=1;
       poly_dec=alloue_poly(poly->nbp,14);
       constitue_poly_decouple(poly,poly_dec,1);
       epure_polygone_face(face,poly_dec,pcont);  /* recursif */
       poly_dec=alloue_poly(poly->nbp,15);
       constitue_poly_decouple(poly,poly_dec,2);
       epure_polygone_face(face,poly_dec,pcont);  /* recursif */
       desalloue_poly(poly);
    }
else
    {if(poly->nbp>3){cir=copy_poly_cir(poly);
                     if(cir->nbp>3)
                           {fenetre_circuit(cir);
                            for(i=0;i<4;i++)cir->vnorm[i]=face->vnorm[i];
                            ajoute_cir_pcont(cir,pcont);
                           }
                         else desalloue_circuit(cir,392);

                    }
     desalloue_poly(poly);
    }

}
/******************************************************************/
struct spol *copy_cir_poly(cir)
struct circuit *cir;
{int i; 
struct spol *polyc;
   polyc=alloue_poly(cir->nbp,16);
   polyc->nbp=cir->nbp;
   for(i=0;i<polyc->nbp;i++)
     {polyc->x[i]=cir->x[i]; polyc->y[i]=cir->y[i]; polyc->z[i]=cir->z[i];}
   return(polyc);
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 1 traitement des contours supports par rapport aux supports devants      */  
/*                           traitement NOT                         	          */  
/*--------------------------------------------------------------------------------*/
int etape1_moins(resul_debut,resul_cour,pcont,pcont1)
struct contour **resul_debut,*pcont,**resul_cour,*pcont1;
{struct circuit *pcir,*pci;
 struct contour *resul,*res;
 int ind;

  copie_cir(pcont1->debut_support,&cir1);

  resul=*resul_cour;
  pcir=pcont->debut_support; 
  copie_cir(pcir,&cir0);
 /* alloue un contour resultat pour chaque operation de "masque" */
   res=alloue_contour(6); 
   if(res==NULL)pb_allocation(40);
  /* printf("\n\n ETAPE 1 resul_debut = %ld resul_cour = %ld \n\n",*resul_debut,*resul_cour);*/

  if(surface(pcir->nbp,pcir->x,pcir->y)>0)
	{/*printf("\n cir0 dans le mauvais sens\ etape1 \n");
	 liste_circuit_ecran(pcir); */
        }

  ind=masque(res,2); /* NOT */ 
  /* ind=-1  ils sont disjoints on s'en va */
  if(ind==1) /* cir1 est inclus dans cir0 */
	{/* on met le circuit cir0 dans le premier contour resultat */
	 /* en lui ajoutant cir1 comme vide */
         *resul_debut=alloue_contour(7); 
         if(*resul_debut==NULL)pb_allocation(41);
	 resul=*resul_debut; 
	 pcir=alloue_circuit(13); if(pcir==NULL)pb_allocation(42);
	 copie_cir(&cir0,pcir);	resul->debut_support=pcir;
	 pcir=alloue_circuit(14); if(pcir==NULL)pb_allocation(43);
         copie_cir(&cir1,pcir);	
         if(option_calcul_z)calculz(&cir0,pcir);
         resul->debut_interieur=pcir;
	  /* on ajoute les trous de cir1 dans les contours resultats */
        pci=pcont1->debut_interieur;
         while(pci)
	  {resul->suc=alloue_contour(8);
           if(resul->suc==NULL)pb_allocation(44);
	   resul=resul->suc;
	   pcir=alloue_circuit(15); if(pcir==NULL)pb_allocation(45);
	   copie_cir(pci,pcir); 

	   /* calcul des z des trous ajoutes */
 	   if(option_calcul_z)calculz(&cir0,pcir);
       	   resul->debut_support=pcir; 
	   pci=pci->suc;
	  }
	 /* on ne fait pas l'etape 2 + traitement particulier pour le 1er support */
	}
	/* ind=2   le support peut etre vu par un trou de cir1 */
        /* on enchaine sur l'etape 2 */
	
 else if(ind==0)
	{ pcir=res->debut_support; 
	 while(pcir)
	    {pci=pcir->suc;
             if(*resul_debut==NULL)
               {*resul_debut=alloue_contour(9); 
		if(*resul_debut==NULL)pb_allocation(46);
	        resul=*resul_debut; 
	       }
	     else
	       {resul->suc=alloue_contour(10); 
		if(resul->suc==NULL)pb_allocation(47);
	        resul=resul->suc;  
	       }
              
             resul->debut_support=alloue_circuit(16);
             if(resul->debut_support==NULL)pb_allocation(48); 
	     copie_cir(pcir,resul->debut_support); 
             resul->debut_support->suc=NULL;
	    pcir=pci;
            }

       }
 desalloue_chaine_contour(res,1024); res=NULL;
 *resul_cour=resul;

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);

/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
 return(ind);
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 2 traitement des contours supports a travers les trous du contour devant */ 
/*     traitement AND                                                             */
/*--------------------------------------------------------------------------------*/
int etape2_moins(pcont,resul_debut,resul_cour,pcdev)
struct contour *pcont,**resul_debut,**resul_cour,*pcdev;
{struct contour *pc,*resul,*res;
 struct circuit *pci,*pcir;
 int ind;
/* printf("\n\n ETAPE 2 resul_debut = %ld resul_cour = %ld \n\n",*resul_debut,*resul_cour);  
*/
 pc=pcdev;
 resul=*resul_cour;
 copie_cir(pcont->debut_support,&cir0);
  ind=0;
  /* alloue un contour resultat pour chaque operation de "masque" */
    res=alloue_contour(11);   if(res==NULL)pb_allocation(49);

      pci=pc->debut_interieur;
      while(pci)
       {copie_cir(pci,&cir1);
               init_res(res);
        ind=masque(res,3); /* AND */ 
     /*  printf("\n RESULTAT ETAPE 2 ind = %d ",ind);*/
       
       if(ind==1)
	    {if(*resul_debut==NULL)
                         {resul=alloue_contour(12);
		          if(resul==NULL)pb_allocation(400);
		          *resul_debut=resul; 
			 }
             else 
		{resul->suc=alloue_contour(13);
                 if(resul->suc==NULL)pb_allocation(401);
		  resul=resul->suc;
	        }
             resul->suc=NULL; 
	     resul->debut_support=alloue_circuit(17);
	     if(resul->debut_support==NULL)pb_allocation(402); 
             pcir=resul->debut_support;
             copie_cir(&cir1,pcir); 
            /* calcul des z  */
            if(option_calcul_z)calculz(&cir0,pcir);
	   }

        else if(ind==2)/* le contour derriere est entierement vu dans un trou return */
           { desalloue_chaine_contour(res,1000); res=NULL; return(2);}
                      
	else if(ind==0)
		{pcir=res->debut_support;
                 while(pcir)
                  {if(*resul_debut==NULL)
                         {resul=alloue_contour(14);
                          if(resul==NULL)pb_allocation(403);
			  *resul_debut=resul; 
                         }
                    else {resul->suc=alloue_contour(15); 
		          if(resul->suc==NULL)pb_allocation(416);
                          resul=resul->suc; 
                         }
                    resul->suc=NULL; 
                    resul->debut_support=alloue_circuit(18);
                    if(resul->debut_support==NULL)pb_allocation(404); 
		   copie_cir(pcir,resul->debut_support);
                    pcir=pcir->suc;
		  }
	       }
        pci=pci->suc;
        desalloue_chaine_contour(res,1001);
        res=alloue_contour(16);if(res==NULL)pb_allocation(49);
     }
 *resul_cour=resul;
 desalloue_chaine_contour(res,1002);  res=NULL;

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);


/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
 return(ind);
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 3 traitement des parties vues des contours supports par rapport a ses    */
/* trous originaux         (traitement NOT)                                       */ 
/*--------------------------------------------------------------------------------*/
int etape3_moins(pcont,resul_debut)
struct contour *pcont,**resul_debut;
{struct contour *pc,*pcsuc,*pcpred,*res,*pco,*pct,*pctsuc;
 struct circuit *pcir,*pci,*pcii,*pcc,*pcirpred,*pcis,*pcio;
 int ind,k,idedan,isur;
 pci=pcont->debut_interieur;
 /* printf("\n\n ETAPE 3  pci = %ld \n\n",pci);*/
 if(pci==NULL)return(0);
 while(pci)
   {copie_cir(pci,&cir1);
    pc=*resul_debut; pcpred=NULL;
    while(pc)
      {pcir=pc->debut_support; pcsuc=pc->suc;
       copie_cir(pcir,&cir0);
      /* alloue un contour resultat pour chaque operation de "masque" */
       res=alloue_contour(17);  if(res==NULL)pb_allocation(405);

 if(surface(pcir->nbp,pcir->x,pcir->y)>0)
	{/*printf("\n cir0 dans le mauvais sens\ etape3 \n");
	 liste_circuit_ecran(pcir); */
        }

       ind=masque(res,2); /* NOT */ 
      /*  printf("\n etape 3 = %d ",ind);   */
        if(ind==1)/* le trou est inclus dans le support */
          {/* on ajoute le trou au contour pc et on passe au traitement du trou suivant */ 
           if(pc->debut_interieur==NULL)
		{pcir=alloue_circuit(19);
	         if(pcir==NULL)pb_allocation(406); 
                 pc->debut_interieur=pcir;}
	   else {pcir=pc->debut_interieur;while(pcir->suc)pcir=pcir->suc;
		 pcir->suc=alloue_circuit(20);
	         if(pcir->suc==NULL)pb_allocation(407);
                 pcir=pcir->suc;  
		}
	   copie_cir(&cir1,pcir);
         }
      /* else if(ind==-1) ils sont disjoints */
       else if(ind==0)
          {/* on insere le support modifie dans la liste des supports */
  	    copie_cir(res->debut_support,pc->debut_support);  
	    pcir=res->debut_support->suc;  
	   pco=pc;
	   while(pcir)
		{pc->suc=alloue_contour(18); 
                 if(pc->suc==NULL)pb_allocation(408); 
                 pc=pc->suc;
                 pc->debut_support=alloue_circuit(21);
                 if(pc->debut_support==NULL)pb_allocation(409); 
                 copie_cir(pcir,pc->debut_support);
                 pcir=pcir->suc;
		}
	    pc->suc=pcsuc;
           if(res->debut_support->suc) /* le trou a decoupe le support en plusieurs parties */
				       /* il faut reaffecter les trous deja places */
		{ /*printf("\n le trou a decoupe le support en plusieurs parties \n");*/
   		pcir=pco->debut_interieur;  pcirpred=NULL;
                while(pcir)
	           {/* cherche si un point de pcir interieur a un support */
                    /* si interieur break; */
  		    /* si exterieur il faut chercher a quel support le trou appartient */
		    /* et le supprimer dans pc->debut_support */
                    pcis=pcir->suc;
		    pcio=pco->debut_support;
		   /* cherche sur le premier support */
		    for(k=0;k<pcir->nbp-1;k++)
			{inpoly(&idedan,&isur,pcir->x[k],pcir->y[k],pcio);
			 if(idedan==1)break;
			}
		    if(idedan!=1)
		     {pct=pco->suc; 
                      while(pct)
		      { pctsuc=pct->suc;
                       pcio=pct->debut_support;
		       for(k=0;k<pcir->nbp-1;k++)
			{inpoly(&idedan,&isur,pcir->x[k],pcir->y[k],pcio);
			 if(idedan==1) /* on ajoute le trou au contour pc et on le supprime a pco */
				{pcii=alloue_circuit(22);
				 if(pcii==NULL)pb_allocation(410); 
			  	 copie_cir(pcir,pcii);  
                                 if(pct->debut_interieur==NULL)pct->debut_interieur=pcii;  
				 else { pcc=pct->debut_interieur; 
				        while(pcc->suc){pcc=pcc->suc;}
					pcc->suc=pcii;
				      }
			         if(pcirpred==NULL)pco->debut_interieur=pcir->suc;
				 else pcirpred->suc=pcir->suc;
			         desalloue_circuit(pcir,6); pcir=pcirpred; pctsuc=0;
				 break;
				}
			}
                       pct=pctsuc;
		      }
		     }
		     pcirpred=pcir; pcir=pcis; 
		   }
		}/* fin du if */
	   }
     else if(ind==2)
         {/* le support est inclus dans le trou -> le contour est a supprimer */
	   if(pcpred==NULL)*resul_debut=pc->suc;
	   else pcpred->suc=pc->suc;
           
           desalloue_contour(pc,10);   pc=NULL;
	 }
      if(ind==1)pc=NULL;
      else {pcpred=pc; pc=pcsuc;}
      desalloue_chaine_contour(res,1003); res=NULL;
     }
   pci=pci->suc;
  }

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);

/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/

}
/*--------------------------------------------------------------------------------*/
/* ETAPE 4 traitement du vide du contour support par rapport a ses                */
/* trous originaux         (traitement OR)                                        */ 
/*--------------------------------------------------------------------------------*/
int etape4_moins(pcont,resul_debut)
struct contour *pcont,*resul_debut;
{struct circuit *pc,*pcour,*pci,*pcir,*pcirsuc,*pcirpred;
 struct contour *res,*pcc;
 int ind,trouve,k,isur,idedan;
  /* printf("\n\n ETAPE 4  \n\n");*/
 /* prend le vide de l'etape 1 */
   pci=resul_debut->debut_interieur; pcour=pci;
   copie_cir(pci,&cir1);
   ind=0;
 /* le compare aux trous des supports derrieres vus */
     pc=pcont->debut_interieur;
 /* alloue un contour resultat pour chaque operation de "masque" */
     res=alloue_contour(19); if(res==NULL)pb_allocation(411);
     while(pc)
       {copie_cir(pc,&cir0);
        ind=masque(res,1); /* OR */
      /*  printf("\n ind = %d ",ind);*/
 /* ind=-1 ils sont disjoints  on ajoute le trou au support  */
 /* ind==1 le vide est inclus dans le trou->le contour devant et derriere sont disjoints */
 /*         le contour derriere est inchange */
        if(ind==-1) 
          {pcour->suc=alloue_circuit(23);
           if(pcour->suc==NULL)pb_allocation(412);
           pcour=pcour->suc; 
	   pcour->suc=NULL; copie_cir(&cir0,pcour);
	  }
        if(ind==1)break;
    /* ind=2 le trou est inclus dans le vide  -> on n'ajoute pas le trou et le vide est inchange */
   /* ind=0 le vide est agrandi -> on n'ajoute pas le trou et on modifie le vide cir1 */
        
        else if(ind==0) 
		{if(res->debut_support->suc==NULL)copie_cir(res->debut_support,&cir1);
		 else  /*il faut chercher le polygone englobant */
		     {pcir=res->debut_support;  trouve=0;
		      while(pcir)
			{if(trouve==1||surface(pcir->nbp,pcir->x,pcir->y)>0)
                          {invsens(pcir);/* on ajoute le circuit comme contour supplementaire */
                           pcc=resul_debut; while(pcc->suc)pcc=pcc->suc;
            		   pcc->suc=alloue_contour(20); 
			   if(pcc->suc==NULL)pb_allocation(414);
			   pcc=pcc->suc;
                           pcc->debut_support=alloue_circuit(25);
			   if(pcc->debut_support==NULL)pb_allocation(415);
			   copie_cir(pcir,pcc->debut_support);
			  }
			 else
			  {/* c'est le trou englobant */
			   copie_cir(pcir,&cir1);
			   trouve=1;
			  }
			 pcir=pcir->suc;
			}
		     }
                 /* test si un trou deja place n'est pas inclus dans le vide */
		  pcir=resul_debut->debut_interieur->suc; 
      		  pcirpred=resul_debut->debut_interieur;
		  while(pcir)
		   {k=0;  pcirsuc=pcir->suc;
		    while(k<pcir->nbp-1)
			{inpoly(&idedan,&isur,pcir->x[k],pcir->y[k],&cir1);
               		 if(idedan==1)
				{/* le point est interieur-on supprime le trou */
				 pcirpred->suc=pcirsuc;
				 desalloue_circuit(pcir,112); break;
				}
			 else if(isur==0)/* le point est exterieur */
				{pcirpred=pcir; break;}
			  /*  le point est sur contour - on passe au point suivant */
			  k++;
			}
                   if(pcir && k==pcir->nbp){printf(" pb - union "); pcirpred=pcir;}
		   pcir=pcirsuc;
	          }
		}
        pc=pc->suc;
        desalloue_chaine_contour(res,1004);
        res=alloue_contour(21); if(res==NULL)pb_allocation(411);
       
       }
  desalloue_chaine_contour(res,1005);  res=NULL;
  /*sprintf(imp,"\n ind = %d ");imprime();*/
  if(ind!=1)/* on ajoute le vide aux trous du support */
    {pcour=resul_debut->debut_interieur; pci=pcour->suc; 
     copie_cir(&cir1,pcour);  pcour->suc=pci;
    }
 

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);

/*
 res=resul_debut; printf("\n res = %ld ",res);
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
  return(ind);
}
/*-----------------------------------------------------------------------------------*/
/*                                    FACE INTER FACE				     */
/*-----------------------------------------------------------------------------------*/
int face1_inter_face2(face2,proj2,face1,proj1,face_resul,proj)

struct modelisation_face *face1,*face2,*face_resul;
int proj1,proj2,proj;

{struct contour *pcont1,*pcont2;
 struct contour *pcont,*pcdev,*pc,*pcontcour,*resul_debut,*resul_cour;
 struct contour *contour_resul,*contour_resul_cour,*pc_face_resul;
 int ind,id;

/*printf(" face1_inter_face2 \n");*/

		init_cir(&cir0);
		init_cir(&cir1);


           /* liste_face(face2,proj2);liste_face(face1,proj1);*/
            /* test si face1 est nulle */
	   ind=0; id=0;
           if(proj1){if(face1->debut_projete==NULL)ind=1;}
	   else{if(face1->debut_dessin==NULL)ind=1;}
	   if(ind==0)id=recoup_circuit(face1->fen,face2->fen);
	    /* test les fenetres englobantes */
           if(ind==1||id==-1)
	     {if(proj){if(face_resul->debut_projete)desalloue_chaine_contour(face_resul->debut_projete,1006);
		       face_resul->debut_projete=NULL;
		      }
	      else    {if(face_resul->debut_dessin)desalloue_chaine_contour(face_resul->debut_dessin,1007);
		       face_resul->debut_dessin=NULL;
		      }
	      return(-1);
	     }
	
           if(proj1)pcont1=face1->debut_projete;
           else pcont1=face1->debut_dessin;
           if(proj2)pcont2=face2->debut_projete;
           else pcont2=face2->debut_dessin;
           pc_face_resul=NULL;

           pcdev=pcont1;
           while(pcdev)
              {pcont=pcont2; contour_resul=NULL;
               /*printf("\n pcdev = %ld  suc = %ld \n",pcdev,pcdev->suc);*/
               while(pcont)
                     {resul_debut=NULL; resul_cour=NULL; 
                      ind=etape1_inter(&resul_debut,&resul_cour,pcont,pcdev);
/* printf(" etape_1inter ind = %d \n",ind); */
	          /* on enleve les trous au resultat */
		      if(ind==1)etape3_inter(pcont,&resul_debut);
                      else if(ind==2)etape3_inter(pcdev,&resul_debut);
		      else if(ind==0)
			{etape3_inter(pcdev,&resul_debut); 
		         etape3_inter(pcont,&resul_debut); 
		        }

                      if(ind!=-1)
		       {/* union des trous des contours resultats */
		         etape4_inter(resul_debut); 
			/* insere le resultat dans les contours de la face */

			if(resul_debut!=NULL) 
		            {pc=resul_debut;
			     if(contour_resul==NULL)contour_resul=pc;
			     else pcontcour->suc=pc;
			     while(pc->suc)pc=pc->suc;   pcontcour=pc;
			    }
		       }
		    /* prend le contour suivant de la face */
		    pcont=pcont->suc;
                  }
             if(contour_resul!=NULL)
                  {if(pc_face_resul==NULL)pc_face_resul=contour_resul;
                   else contour_resul_cour->suc=contour_resul;
                   pcont=contour_resul;
		   while(pcont->suc)pcont=pcont->suc;
                   contour_resul_cour=pcont;
                  }
                pcdev=pcdev->suc;
	      }


   if(proj){if(face_resul->debut_projete)desalloue_chaine_contour(face_resul->debut_projete,1008);
	    face_resul->debut_projete=pc_face_resul;
	   }
   else   {if(face_resul->debut_dessin)desalloue_chaine_contour(face_resul->debut_dessin,1009);
	   face_resul->debut_dessin=pc_face_resul;
	  }
	    
    fen_face(face_resul,proj);

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);

}

/*--------------------------------------------------------------------------------*/
/* ETAPE 1 traitement supports-supports  				          */  
/*                traitement AND                                  	          */  
/*--------------------------------------------------------------------------------*/
int etape1_inter(resul_debut,resul_cour,pcont,pcont1)
struct contour **resul_debut,*pcont,**resul_cour,*pcont1;
{struct circuit *pcir,*pci;
 struct contour *resul,*res,*pc;
 int ind;

   copie_cir(pcont1->debut_support,&cir1);

  resul=*resul_cour;
  pcir=pcont->debut_support; 
  copie_cir(pcir,&cir0);
 /* alloue un contour resultat pour chaque operation de "masque" */
   res=alloue_contour(6); 
   if(res==NULL)pb_allocation(40);
   /*printf("\n\n ETAPE 1 resul_debut = %ld resul_cour = %ld \n\n",*resul_debut,*resul_cour);*/
   ind=masque(res,3); /* NOT */ 
  /*printf("\n RESULTAT ETAPE 1 ind = %d ",ind);*/
  /* ind=-1  ils sont disjoints on s'en va */
  if(ind==1) /* cir1 est inclus dans cir0 */
	{/* on met le circuit cir1 dans le premier contour resultat */
            *resul_debut=copie_contour(pcont1,*resul_debut);
	    if(option_calcul_z)
	     {pc=*resul_debut;
	      while(pc)
	       {pci=pc->debut_support;
                while(pci){ calculz(&cir0,pci);  pci=pci->suc; }
                pci=pc->debut_interieur;
                while(pci){ calculz(&cir0,pci);  pci=pci->suc; }
	        pc=pc->suc;
	       }
             }
        }
  else if(ind==2)
	{/* on met le circuit cir0 dans le premier contour resultat */
            *resul_debut=copie_contour(pcont,*resul_debut);
	}
  else if(ind==0)
	{pcir=res->debut_support; 
	 while(pcir)
	    {pci=pcir->suc;
             if(*resul_debut==NULL)
               {*resul_debut=alloue_contour(9); 
		if(*resul_debut==NULL)pb_allocation(46);
	        resul=*resul_debut; 
	       }
	     else
	       {resul->suc=alloue_contour(10); 
		if(resul->suc==NULL)pb_allocation(47);
	        resul=resul->suc;  
	       }
             resul->debut_support=alloue_circuit(16);
             if(resul->debut_support==NULL)pb_allocation(48); 
	     copie_cir(pcir,resul->debut_support); 
             resul->debut_support->suc=NULL;
	    pcir=pci;
            }
       }
 desalloue_chaine_contour(res,1010); res=NULL;
 *resul_cour=resul;

/*

		      desalloue_point_circuit_cir(&cir0,1);
		      desalloue_point_circuit_cir(&cir1,2);
*/
/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
 return(ind);
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 3 traitement des parties vues des contours supports par rapport aux      */
/* trous de cir0 et de cir1         (traitement NOT)                              */
 /*-------------------------------------------------------------------------------*/
int etape3_inter(pcont,resul_debut)
struct contour *pcont,**resul_debut;
{struct contour *pc,*pcsuc,*pcpred,*res,*pco;
 struct circuit *pcir,*pci,*pcirpred,*pcirsuc;
 int ind,id;
 pci=pcont->debut_interieur;
/* printf("\n\n ETAPE 3  pci = %ld \n\n",pci);*/
 if(pci==NULL)return(0);
 while(pci)
   {copie_cir(pci,&cir1);
    pc=*resul_debut; pcpred=NULL;
    while(pc)
      {pcir=pc->debut_support; pcsuc=pc->suc;
       copie_cir(pcir,&cir0);
      /* alloue un contour resultat pour chaque operation de "masque" */
       res=alloue_contour(17);  if(res==NULL)pb_allocation(405);
       ind=masque(res,2); /* NOT */ 
     /*  printf("\n ind = %d \n",ind);*/


       if(ind==1)/* le trou est inclus dans le support */
         {/* on ajoute le trou au contour pc et on passe au traitement du trou suivant */ 
           if(pc->debut_interieur==NULL)
		{pcir=alloue_circuit(19);
	         if(pcir==NULL)pb_allocation(406); 
                 pc->debut_interieur=pcir;}
	   else {pcir=pc->debut_interieur;while(pcir->suc)pcir=pcir->suc;
		 pcir->suc=alloue_circuit(20);
	         if(pcir->suc==NULL)pb_allocation(407);
                 pcir=pcir->suc;  
		}

	   copie_cir(&cir1,pcir);
	   if(option_calcul_z)calculz(&cir0,pcir);

         }
      /* else if(ind==-1) ils sont disjoints */
       else if(ind==0)
          {/* on insere le support modifie dans la liste des supports */
  	    copie_cir(res->debut_support,pc->debut_support);  
	    pcir=res->debut_support->suc;  pco=pc; 
	    while(pcir)
		{pco->suc=alloue_contour(18); 
                 if(pco->suc==NULL)pb_allocation(408); 
                 pco=pco->suc;
                 pco->debut_support=alloue_circuit(21);
                 if(pco->debut_support==NULL)pb_allocation(409); 
                 copie_cir(pcir,pco->debut_support);
                 pcir=pcir->suc;
		}
	    /* si le support decoupe comportait deja des trous il faut */
		 /* aussi decouper les trous */
                 pcirpred=NULL;
             /* copie des trou dans une chaine de circuits */
                 pcir=copie_trou_circuits(pc->debut_interieur);
		 pc->debut_interieur=NULL;
                 while(pcir)
                   {pcirsuc=pcir->suc;
		    copie_cir(pcir,&cir0);
		    desalloue_chaine_contour(res,1011);
		    res=alloue_contour(17);  if(res==NULL)pb_allocation(405);
                    id=masque(res,2); 
                    desalloue_circuit(pcir,383);
                    /* si id=1 ou id=2 les 2 trous sont identiques et en bordure du support - on ne fait rien */
                    if(id==-1)
	 		 {res->debut_support=alloue_circuit(123);
			  if(res->debut_support==NULL)pb_allocation(123);
			  copie_cir(&cir0,res->debut_support);
			  affecte_trou(res,pc);
			 }
		    else if(id==0)
			{/* on reaffecte les trous decoupes */
			 affecte_trou(res,pc);
			}
                     pcir=pcirsuc;
		   }
            pc=pco;  pc->suc=pcsuc;
	   }
     else if(ind==2)
         {/* le support est inclus dans le trou -> le contour est a supprimer */
	   if(pcpred==NULL)*resul_debut=pc->suc;
	   else pcpred->suc=pc->suc;
           desalloue_contour(pc,10); pc=NULL;
	 }
      if(ind==1)pc=NULL;
      else {pcpred=pc; pc=pcsuc;} 


      desalloue_chaine_contour(res,1012); res=NULL;
     }
   pci=pci->suc;
  }

		      desalloue_point_circuit_cir(&cir0,1);
		      desalloue_point_circuit_cir(&cir1,2);

/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
}
/*--------------------------------------------------------------------------------*/
/* ETAPE 4 union des trou du contour resultat                                     */
/*                         (traitement OR)                                        */ 
/*--------------------------------------------------------------------------------*/
int etape4_inter(resul_debut)
struct contour *resul_debut;
{struct circuit *ptraite,*ptraitesuc,*ptraite_pred,*pcisuc,*pci,*pc,*pcpred,*pcsuc,*pcour;
 struct circuit *cir_traite,*pcipred;
 struct contour *res;
 int ind;
/* printf("\n\n ETAPE 4 %ld  \n\n",resul_debut);*/
  if(resul_debut==NULL||resul_debut->debut_interieur==NULL||resul_debut->debut_interieur->suc==NULL)return(0);

   pci=resul_debut->debut_interieur; pcipred=NULL;
   while(pci&&pci->suc)
   {cir_traite=alloue_circuit(87);
     if(cir_traite==NULL)pb_allocation(87);
     copie_cir(pci,cir_traite); cir_traite->suc=NULL;
     pcisuc=pci->suc;
     pc=pci->suc; pcpred=pci;
     while(pc)
        {pcsuc=pc->suc;
         ptraite=cir_traite; ptraite_pred=NULL; 
	 while(ptraite)
	    {ptraitesuc=ptraite->suc;
             copie_cir(ptraite,&cir0);
	     copie_cir(pc,&cir1);
	     ind=cir_union_cir(&res);
             if(ind==1)
		{desalloue_chaine_contour(res,1014);
		 pcpred->suc=pcsuc; 
		 if(pc==pcisuc)pcisuc=pcsuc;
                 desalloue_circuit(pc,235);
                 pc=pcpred;
		 ptraitesuc=NULL;
	        }
  	     else if(ind==3)
		{copie_cir(res->debut_support,pc); pc->suc=pcsuc;
                 if(ptraite_pred==NULL)cir_traite=ptraitesuc;
		 else ptraite_pred->suc=ptraitesuc;
	         desalloue_circuit(ptraite,432);
		  ptraite=ptraite_pred;
		}
	     else if(ind==0)
	        {if(ptraite_pred==NULL)cir_traite=res->debut_support;
                 else ptraite_pred->suc=res->debut_support;
                    pcour=res->debut_support;
	            while(pcour->suc)pcour=pcour->suc;
                    pcour->suc=ptraitesuc;
	        desalloue_circuit(ptraite,125);
                ptraite=pcour;
	       }
             else if(ind==2)
	       {desalloue_chaine_contour(res,1014);
		if(ptraite_pred==NULL)cir_traite=ptraitesuc;
		else ptraite_pred->suc=ptraitesuc;
		desalloue_circuit(ptraite,129);
		ptraite=ptraite_pred;
	       }
             else if(ind==-1)
	       {
		desalloue_chaine_contour(res,1014);
	       }

             ptraite_pred=ptraite;  ptraite=ptraitesuc;
	    }
	 pcpred=pc; pc=pcsuc;
        }
    if(cir_traite==NULL)
	{if(pcipred==NULL)resul_debut->debut_interieur=pcisuc;
         else pcipred->suc=pcisuc;
	 desalloue_circuit(pci,654);
	 pci=pcipred;
        }
    else 
	{if(pcipred==NULL)resul_debut->debut_interieur=cir_traite;
         else pcipred->suc=cir_traite;
         pcour=cir_traite;
	 while(pcour->suc)pcour=pcour->suc;
	 pcour->suc=pcisuc;
	 desalloue_circuit(pci,655);
         pci=pcour;
        }
    pcipred=pci; pci=pcisuc; 
  }

 /* modif du 19/03/96 desalloue remonte dans if ind==1 -1 et 2       */
 /* if(ind!=0&&ind!=3){desalloue_chaine_contour(res,1013); res=NULL;}*/

pci=resul_debut->debut_interieur;
  return(ind);
}
/*-------------------------------------------------------------------------------*/
int cir_union_cir(resultat)
struct contour **resultat;
{struct contour *res;
 int ind,id; 

       res=alloue_contour(1999); if(res==NULL)pb_allocation(411);
       ind=masque(res,1); /* OR */
       id=-2;
        if(ind==0) 
		{if(res->debut_support->suc!=NULL)
                        { /*on soustrait les 2 trous */
		 	 desalloue_chaine_contour(res,1014);
	      	 	  res=alloue_contour(191); if(res==NULL)pb_allocation(401);
			  id=masque(res,2);
			  if(id==-1)ind=-1;
		         /* il y a obligatoirement une intersection ou disjoints */
                          if(id!=0&&id!=-1)
				{printf(" moins etape3 - probleme \n");exit(0);}
		        }
		}
     *resultat=res;
     if(ind==0 && id==-2)ind=3;
     return(ind);
}

/*----------------------------------------------------------*/
struct circuit *support_trou(pcir1,pcir2)
struct circuit *pcir1,*pcir2;
{struct circuit *pres_debut,*pcour,*pci,*pcir;
 struct contour *res;
 int ind;
   /* printf("\n support trou \n");*/
    copie_cir(pcir1,&cir1); pres_debut=NULL;
    pcir=pcir2;
    res=alloue_contour(199); if(res==NULL)pb_allocation(491);
    while(pcir)
	{init_res(res);
         copie_cir(pcir,&cir0);
	 ind=masque(res,2);  /*  NOT */
	 /* ind=2 le support disparait */
	  if(ind==1){printf("\n support_trou  ind=1 - impossible \n");exit(0);}
	  else if(ind==0)
	      {pci=res->debut_support;
		while(pci)
		 {if(pres_debut==NULL)
		    {pres_debut=alloue_circuit(22); if(pres_debut==NULL)pb_allocation(22);
		     copie_cir(pci,pres_debut); pcour=pres_debut;}
		  else {pcour->suc=alloue_circuit(23);
			copie_cir(pci,pcour->suc);
                        pcour=pcour->suc;
                       }
		  pci=pci->suc;
		 }
	      }
          else if(ind==-1)
	   {if(pres_debut==NULL)
	       {pres_debut=alloue_circuit(22); if(pres_debut==NULL)pb_allocation(22);
		 copie_cir(&cir0,pres_debut); pcour=pres_debut;
	       }
	     else {pcour->suc=alloue_circuit(23);
		   copie_cir(&cir0,pcour->suc);
                   pcour=pcour->suc;
                  }
	   }
         desalloue_chaine_contour(res,1015);
         res=alloue_contour(199); if(res==NULL)pb_allocation(491);
	 pcir=pcir->suc;
	}
    
  desalloue_chaine_contour(res,1016); res=NULL;

		      desalloue_point_circuit_cir(&cir0,1);
		      desalloue_point_circuit_cir(&cir1,2);

  return(pres_debut);
}
/*------------------------------------------------------------------*/
int affecte_trou(trou,sup)
struct contour *trou,*sup;
{int idedan,isur,k;
 struct circuit *pci,*pcii,*pcir;
 struct contour *pc;
  pci=trou->debut_support; 
  while(pci)
   {pc=sup;
    while(pc)
     {pcir=pc->debut_support;
      for(k=0;k<pci->nbp-1;k++)
	{inpoly(&idedan,&isur,pci->x[k],pci->y[k],pcir);
	 if(idedan==1||isur==1)
	  {pcii=alloue_circuit(234);
	   copie_cir(pci,pcii); pcii->suc=pc->debut_interieur;
	   pc->debut_interieur=pcii;
	   break;
	  }
	}
       if(k<pci->nbp-1)break; 
       pc=pc->suc;
     }
    pci=pci->suc;
   }
}
/*------------------------------------------------------------------------------------*/
struct circuit *copie_trou_circuits(pci)
struct circuit *pci;
{ struct circuit *pcii,*pcisuc,*psuc;
  psuc=NULL;
  while(pci)
   {pcisuc=pci->suc;
    pcii=alloue_circuit(345); if(pcii==NULL)pb_allocation(345);
    copie_cir(pci,pcii);
    desalloue_circuit(pci,476);
    pcii->suc=psuc;   psuc=pcii;
    pci=pcisuc;
   }
  return(psuc);
}



/*-----------------------------------------------------------------------------------*/
/*                                    FACE UNION FACE				     */
/*-----------------------------------------------------------------------------------*/
int face1_union_face2(face2,proj2,face1,proj1,face_resul,proj)

struct modelisation_face *face1,*face2,*face_resul;
int proj1,proj2,proj;

{struct contour *pcont1,*pcont2;
 struct contour *contour_resul1,*contour_resul2,*pc;
 int ind,id;

		init_cir(&cir0);
		init_cir(&cir1);

           /* test si face1 est nulle */
           ind=0; id=0;
           if(proj1){if(face1->debut_projete==NULL)ind=1;}
           else{if(face1->debut_dessin==NULL)ind=1;}
		
	   if(ind==0)id=recoup_circuit(face1->fen,face2->fen);

            /* test les fenetres englobantes */
	   
          if(proj1)pcont1=face1->debut_projete;
           else pcont1=face1->debut_dessin;
           if(proj2)pcont2=face2->debut_projete;
           else pcont2=face2->debut_dessin;
         /* union des 2 chaines de contour separement */

/*** 
les deux union_chaine_contour devraient se faire en amont de la
procedure face1_union_face2. On  eviterait ainsi de repeter les unions de faces
dans certains traitements comme union_face_fic qui amene une face deja traitee

****/


           contour_resul2=union_chaine_contour(pcont2);

           if(ind==0)
		{contour_resul1=union_chaine_contour(pcont1);
		 if(id==-1)
	             {pc=contour_resul2;
		      while(pc->suc)pc=pc->suc; 
		      pc->suc=contour_resul1;
		     }
                 else {
			contour_resul2=chaine_union_chaine(contour_resul1,contour_resul2);
			desalloue_chaine_contour(contour_resul1);
		      }
	        }
   if(proj){if(face_resul->debut_projete)
		  {desalloue_chaine_contour(face_resul->debut_projete,1017);}
             face_resul->debut_projete=contour_resul2;
	   }
   else   {if(face_resul->debut_dessin)
		 {desalloue_chaine_contour(face_resul->debut_dessin,1018);}
	   face_resul->debut_dessin=contour_resul2;
	  }

	desalloue_point_circuit_cir(&cir0,1);
	desalloue_point_circuit_cir(&cir1,1);

    fen_face(face_resul,proj);

    return(id);
    
}
/*--------------------------------------------------------------------------------*/
struct contour *union_chaine_contour(pconti)
struct contour *pconti;
{struct contour *contour_resul,*pc,*pct,*pctsuc,*resul_debut,*pcour,*pctpred;
 struct contour *pcoursuc,*pcourpred,*pcont;

 //TEMPO pour impression
 int i;
 int noTROU,noSUP,noTRAITE;
 struct contour *pcontTEMP;
 struct circuit *pcirTEMP;

      contour_resul=NULL;
      pc=pconti;
	   while(pc)
	    {pcont=NULL;
	     pcont=copie_contour(pc,pcont);
	     if(contour_resul==NULL)contour_resul=pcont;
	     else pcour->suc=pcont;
	     pcour=pcont;
	     pc=pc->suc;
	    }

       pcour=contour_resul;   pcourpred=NULL;
	   while(pcour)
	      {pct=pcour->suc; pctpred=pcour; pcoursuc=pcour->suc;
           while(pct)
					{pctsuc=pct->suc;
					 resul_debut=NULL;
/*
// TEMPO
printf("AVANT Appel contour_union_contour : \n");
printf("traite pcour\n");
  printf("Pcour Support \n"); 
  pcirTEMP=pcour->debut_support;
  for(i=0;i<pcirTEMP->nbp;i++)
  { printf(" %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
  }
  noTROU=0;
  pcirTEMP=pcour->debut_interieur;
  while(pcirTEMP)
  { printf("Pcour TROU %d \n",noTROU+1); 
	for(i=0;i<pcirTEMP->nbp;i++)
	{ printf("  %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
	}
	pcirTEMP=pcirTEMP->suc;
    noTROU++;
  }
printf("avec pct \n");
  printf("Pct Support \n"); 
  pcirTEMP=pct->debut_support;
  for(i=0;i<pcirTEMP->nbp;i++)
  { printf(" %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
  }
  noTROU=0;
  pcirTEMP=pct->debut_interieur;
  while(pcirTEMP)
  { printf("Pct TROU %d \n",noTROU+1); 
	for(i=0;i<pcirTEMP->nbp;i++)
	{ printf("  %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
	}
	pcirTEMP=pcirTEMP->suc;
    noTROU++;
  }
// Fin TEMPO
*/
					 contour_union_contour(pcour,pct,&resul_debut);

					 if(resul_debut->suc==NULL)
						{if(pcour==pctpred)
							{if(pcourpred==NULL)contour_resul=resul_debut;
							 else pcourpred->suc=resul_debut;
							 resul_debut->suc=pctsuc; pcoursuc=resul_debut;
							}
						 else
							{if(pcourpred==NULL)contour_resul=pcour->suc;
							 else pcourpred->suc=pcour->suc;
							 pctpred->suc=resul_debut;
							 resul_debut->suc=pctsuc;
							}
                         desalloue_contour(pcour,54);	
						 desalloue_contour(pct,67);
						 pctsuc=NULL; pcour=pcourpred;
						}
					else
					    {pctpred=pct;
						 desalloue_chaine_contour(resul_debut);
						}
					pct=pctsuc;
				   }
	       pcourpred=pcour;
           pcour=pcoursuc;
	      }

  return(contour_resul);
}
/*--------------------------------------------------------------------------------*/
struct contour *chaine_union_chaine(contour_resul1,contour_resul2)
struct contour *contour_resul1,*contour_resul2;
{struct contour *pc,*resul_debut,*pcour,*pct,*pctsuc,*pctpred;


           pc=contour_resul1;
	   while(pc)
		{pcour=NULL;pcour=copie_contour(pc,pcour);
		 pct=contour_resul2; pctpred=NULL;
	         while(pct)
		  {pctsuc=pct->suc;  resul_debut=NULL;
		   contour_union_contour(pcour,pct,&resul_debut);
                   if(resul_debut->suc==NULL)
			{desalloue_contour(pcour,56);
			 pcour=resul_debut;
			 if(pctpred==NULL)contour_resul2=pctsuc;
			 else pctpred->suc=pctsuc;
			 desalloue_contour(pct,56);
			}
		   else {pctpred=pct; 
		         desalloue_chaine_contour(resul_debut);
		        }
		   pct=pctsuc;
		  }
		 pcour->suc=contour_resul2;
		 contour_resul2=pcour;
		 pc=pc->suc;
		}
           return(contour_resul2);
}
/*--------------------------------------------------------------------------------*/
int contour_union_contour(pcont1,pcont2,resul_debut)
struct contour *pcont1,*pcont2,**resul_debut;
{int ind,id;


// Attention id non initialisé si ind =0;
//	printf("id NON initialise=%d (on initialise à 0)\n",id);
	id=0;

    copie_cir(pcont1->debut_support,&cir1);

    ind=etape1_union(resul_debut,pcont2,pcont1);
	//printf("ETAPE1 : ind = %d\n",ind);
    /* trou de pcont1 - pcont */
    if(ind==1)id=etape3_union(pcont1,pcont2,pcont2,*resul_debut);
    else if(ind==2)id=etape3_union(pcont2,pcont1,pcont2,*resul_debut);
    else if(ind==0)
		{ //printf("Ind=0; Passe à ETAPE3_Union 1\n");
		  etape3_union(pcont2,pcont1,pcont2,*resul_debut);
		  //printf("Ind=0; Passe à ETAPE3_Union 2\n");
          etape3_union(pcont1,pcont2,pcont2,*resul_debut);
        }

	if(ind!=-1||id!=1) // on fait l'intersections des trous des 2 contours (conditions pour étape2 à expliciter)
		{ 
		  etape2_union(pcont2,pcont1,*resul_debut);
		}


}
/*--------------------------------------------------------------------------------*/
/* ETAPE 1 traitement supports-supports  				          */  
/*                traitement OR                                  	          */  
/*--------------------------------------------------------------------------------*/
int etape1_union(resul_debut,pcont,pcont1)
struct contour **resul_debut,*pcont,*pcont1;
{struct circuit *pcir,*pci;
 struct contour *res;
 int ind;
 pcir=pcont->debut_support; 
 copie_cir(pcir,&cir0);
 /* alloue un contour resultat pour chaque operation de "masque" */
   res=alloue_contour(6); 
   if(res==NULL)pb_allocation(40);
  /* printf("\n\n ETAPE 1 resul_debut = %ld \n\n",*resul_debut); */
  ind=masque(res,1); /* OR */ 
  /*  printf("\n RESULTAT ETAPE 1 ind = %d ",ind); */
   if(ind==-1)
	{copie_support_contour(pcont,res);
	 res->suc=alloue_contour(290); if(res->suc==NULL)pb_allocation(290);
	 copie_support_contour(pcont1,res->suc);
         if(option_calcul_z)
		{pci=res->suc->debut_support;
                 while(pci){calculz(&cir0,pci); pci=pci->suc;}
                }
        }
  else if(ind==2) /* cir0 est inclus dans cir1 */
	{/* on met le circuit cir1 dans le premier contour resultat */
	    copie_support_contour(pcont1,res);
        if(option_calcul_z)
		{ pci=res->debut_support;
	      while(pci){ calculz(&cir0,pci); pci=pci->suc; }
		}
    }
  else if(ind==1)
	{ /* on met le circuit cir0 dans le premier contour resultat */
	  copie_support_contour(pcont,res);
	}
  else if(ind==0)
	{if(res->debut_support->suc)
	  {ind=0;  /* c'est un trou definitif */
	   englobant(res);
	  }
    }
  *resul_debut=res;
 
/*
 res=*resul_debut;
 while(res)
    {liste_struct_contour_ecran(res);res=res->suc;}
*/
 return(ind);
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 3 traitement trous de pcont1 moins les  supports de pcont                */
/*          (traitement NOT)                                                      */
/*--------------------------------------------------------------------------------*/
int etape3_union
(pcont,pcont1,pnorm,resul_debut)
struct contour *pcont,*pcont1,*resul_debut,*pnorm;
{struct contour *pc,*pcs,*pco,*pcoi;
 struct circuit *cir_resul,*pct,*pci,*pcis,*cir_traite,*pcour,*pcisuc,*pcipred,*pres;
 int ind;

 //TEMPO pour Impression
 int i;
 int noTROU,noSUP,noTRAITE;
 struct contour *pcontTEMP;
 struct circuit *pcirTEMP;

  if(pcont1->debut_interieur==NULL)return(0);

/*
// TEMPO
  
  printf("Pcont1 Support \n"); 
  pcirTEMP=pcont1->debut_support;
  for(i=0;i<pcirTEMP->nbp;i++)
  { printf(" %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
  }
  noTROU=0;
  pcirTEMP=pcont1->debut_interieur;
  while(pcirTEMP)
  { printf("Pcont1 TROU %d \n",noTROU+1); 
	for(i=0;i<pcirTEMP->nbp;i++)
	{ printf("  %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
	}
	pcirTEMP=pcirTEMP->suc;
    noTROU++;
  }
 
  printf("Pcont avec ses supports \n"); // normalement la gestion ne se fait qu'avec le contour de pcont
  noSUP=0;
  pcontTEMP=pcont;
//  while(pcontTEMP)
//  {	  
	printf("Pcont SUPP %d \n",noSUP+1); 
    pcirTEMP=pcontTEMP->debut_support;
    while(pcirTEMP)
	{
	  for(i=0;i<pcirTEMP->nbp;i++)
	  { printf("  %f %f %f\n", pcirTEMP->x[i],pcirTEMP->y[i],pcirTEMP->z[i]);
	  }
	  pcirTEMP=pcirTEMP->suc;
      noSUP++;
	}
//	pcontTEMP=pcontTEMP->suc;
//  }
// FIN TEMPO
*/
  pc=pcont1;
  cir_resul=NULL; /* debut de la chaine de circuits resultats */


/**** modif du 6 juillet 1996 les unions des supports ont ete realisees 
 simplement sur pcont1 et pcont sans suivre la chaine. Ici il ne faut traiter
 que les trous des supports traites, il ne faut donc pas suivre la chaine 
pcont1 
 
  while(pc)
  {
******/

   noTROU=0;
   pct=pc->debut_interieur;
   while(pct)
   { // compare le trou avec tous les supports de pcont 
	  //printf("Traite TROU %d\n",noTROU+1); noTROU++;
      pcs=pcont;
      cir_traite=alloue_circuit(45);  // debut de la chaine du trous en traitement 
      copie_cir(pct,cir_traite);

	  noSUP=0;
/* MODIF mars 2006
      while(pcs)  // avec chaque contour support de pcont
	  { 
Fin MODIF mars 2006	  */
		//printf(" avec SUPPORT %d\n",noSUP+1); noSUP++;
		pcis=pcs->debut_support;
		copie_cir(pcis,&cir1);
		pci=cir_traite;
		pcipred=NULL;

		noTRAITE=0;
		while(pci)  
		{ 
		  //printf("   avec TROU TRAITE %d\n",noTRAITE+1); noTRAITE++;		
		  pcisuc=pci->suc;
		  copie_cir(pci,&cir0);
	      ind=traite_etape3_union(&pres);

		  if(ind==1) // un support est inclus dans un trou 
		  { pcoi=NULL;
            pcoi=copie_contour(pcs,pcoi);
            pco=resul_debut;
		    while(pco->suc)pco=pco->suc;
            pco->suc=pcoi;
		    pci=NULL;
			//printf("ind = 1\n");
		  }
	      else if(ind==2) // un trou dans un support; supprime le trou
		  { 
			if(pcipred==NULL) cir_traite=pcisuc;
            else pcipred->suc=pcisuc;
	        desalloue_circuit(pci,532);
            pci=pcisuc;
			//printf("ind = 2\n");
	      }
		  else  if(ind==0 ) // support et trou se coupent; insere le resultat dans la chaine 
		  { if(pcipred==NULL)
			{ cir_traite=pres;
			}
		    else pcipred->suc=pres;
		    pcour=pres; 
 		    while(pcour->suc)
			{ pcour=pcour->suc;
			}
		   pcour->suc = pcisuc;
		   pcipred=pcour;
		   desalloue_circuit(pci,533);
		   pci=pcisuc;
		   //printf("ind = 0\n");

		  } // Fin if ind=0
		  else // ind=-1, // support et trou disjoint; trou en traitement non modifié
		  { // MODIF mars 2006
			pcipred=pci;
			// Fin MODIF mars 2006
			pci=pcisuc; 
			//printf("ind = -1\n");
		  }
		} // fin while (pci)
/* MODIF mars 2006
       pcs=pcs->suc;
	  } // fin while (pcs)
Fin MODIF mars 2006 */
     /* insere la chaine cir_traite dans cir_resul */
     if(cir_traite)
	 { //printf("insere cir_traite dans cir_resul\n");
	   if(cir_resul==NULL)
		{ cir_resul=cir_traite;
		}
	   else
	   { pcour=cir_resul;
	     while(pcour->suc)pcour=pcour->suc;
         pcour->suc=cir_traite;
	   }
     }
	 //else printf("rien à insérer\n");
    pct=pct->suc;
   } // fin while (pct)

/****
   pc=pc->suc;
  }
*****/

   /* calcul des z du resultat */
    if(option_calcul_z)
      {if(pnorm!=pcont1)
	{ pcour=cir_resul;   
           while(pcour){calculz(pnorm->debut_support,pcour);pcour=pcour->suc;} 
	}
      }
   /* ajoute les trous calcules au contour resultat */

 pcour=resul_debut->debut_interieur;
 if(cir_resul)
	{if(pcour)
  	  {while(pcour->suc)pcour=pcour->suc;
   	   pcour->suc=cir_resul;
  	  }
 	 else resul_debut->debut_interieur=cir_resul;
	}

return(ind);
}

/*--------------------------------------------------------------------------------*/
// pas utilisé
/*
int egalite (pcs,pcont1)
struct contour *pcs,*pcont1;
{
	double dx, dy, dz;
	double EPSILONE;
	struct circuit *pcir1,*pcir2;
	int i;

	EPSILONE=0.001;

    pcir1=pcs->debut_support;
    pcir2=pcont1->debut_support;
	if(pcir1->nbp != pcir2->nbp) return(0);

	for(i=0;i<pcir1->nbp;i++)
	  { 
		dx =pcir1->x[i]- pcir2->x[i];
		dy =pcir1->y[i]- pcir2->y[i];
		dz =pcir1->z[i]- pcir2->z[i];
		if (fabs(dx) > EPSILONE || fabs(dy) > EPSILONE || fabs(dz) > EPSILONE) return(0);
	  }

printf("egalite\n");
return(1);

}
*/

/*--------------------------------------------------------------------------------*/
int traite_etape3_union(pcir)
struct circuit **pcir;
{struct contour *pc,*res;
 struct circuit *pci;
 int ind;

 int i;

  pci=NULL;
  res=alloue_contour(45); if(res==NULL)pb_allocation(45);

  ind=masque(res,2);
/*
// TEMPO
  if(ind==2)
  { //print cir0 et cir 1
	  printf("cir0\n");
	  for(i=0;i<cir0.nbp;i++)
	  { printf(" %f %f %f \n", cir0.x[i],cir0.y[i],cir0.z[i]); 
	  }
	  printf("cir1\n");
	  /*
	  for(i=0;i<cir1.nbp;i++)
	  { printf(" %f %f %f \n", cir1.x[i],cir1.y[i],cir1.z[i]); 
	  }
  }
// FIN TEMPO
*/
  if(ind!=0)desalloue_chaine_contour(res);

  if(ind==1) /* le support est inclus dans un trou */
    {*pcir=NULL; return(ind);}
  if(res->debut_support!=NULL) 
	{pc=res;
	    while(pc)
		{ if(pci==NULL)pci=pc->debut_support;
	      else
		   { pci->suc=pc->debut_support;pci=pci->suc;

		   }
          pc=pc->suc;
		}
	  libere_contour(res,12);
    }
  *pcir=pci;
  return(ind);
}
/*------------------------------------------------------------------------------------*/
int englobant(res)
struct contour *res;
{struct circuit *pcir,*pcir_eng,*pcir_eng_pred;
 int trouve;
 
      pcir=res->debut_support; pcir_eng_pred=NULL;
	     trouve=0;
	      while(pcir)
		{if(trouve==1||surface(pcir->nbp,pcir->x,pcir->y)>0)invsens(pcir);
                 else
		  {/* c'est le trou englobant */
		   pcir_eng=pcir;
		   trouve=1;
	          }
		 if(trouve==0)pcir_eng_pred=pcir;
                 pcir=pcir->suc;
                }
   /* met l'englobant comme contour support et les autres comme circuits interieurs */
  if(pcir_eng_pred!=NULL)
      {res->debut_interieur=res->debut_support;
       pcir_eng_pred->suc=pcir_eng->suc;
       res->debut_support=pcir_eng;
      }
   else res->debut_interieur=res->debut_support->suc;

   res->debut_support->suc=NULL;             
}

/*--------------------------------------------------------------------------------*/
/* ETAPE 2 traitement trous de pcont1  inter les  trous de pcont                   */
/*          (traitement AND)                                                      */
/*--------------------------------------------------------------------------------*/
int etape2_union(pcont,pcont1,resul_debut)
struct contour *pcont,*pcont1,*resul_debut;
{struct contour *pc;
 struct circuit *cir_resul,*pct,*pci,*cir_traite,*pcour,*pcisuc,*pcipred,*pres;
 struct circuit *cir_trav;
 int ind;



   if(pcont->debut_interieur==NULL || pcont1->debut_interieur==NULL)
								{return(0);}
   //printf("REALISE ETAPE 2\n");

 /* copie des interieurs de pcont dans une chaine de travail */
  pcour=pcont->debut_interieur;
  cir_trav=alloue_circuit(45); 
  pres=cir_trav;
  copie_cir(pcour,pres);
  pcour=pcour->suc; 
  while(pcour)
	{pres->suc=alloue_circuit(46);
	 if(pres->suc==NULL)pb_allocation(46);
	 pres=pres->suc;
         copie_cir(pcour,pres);
	 pcour=pcour->suc;
	}
  cir_traite=cir_trav;

  pc=pcont1;
  cir_resul=NULL; /* debut de la chaine de circuits resultats */

// MODIF mars 2006
//  while(pc)
//  {
// Fin MODIF mars 2006

   pct=pc->debut_interieur;
   while(pct)
   { copie_cir(pct,&cir1);
     /* compare avec tous les supports de pcont */
        pci=cir_traite;  pcipred=NULL;
        while(pci)  
		{ pcisuc=pci->suc;
	      copie_cir(pci,&cir0);
	      ind=traite_etape2_union(&pres);

          if(ind==1)
          { if(cir_resul==NULL)
			{ cir_resul=alloue_circuit(51); if(cir_resul==NULL)pb_allocation(51);
			  copie_cir(pct,cir_resul);
			  if(option_calcul_z) calculz(pcont->debut_support,cir_resul);
			}
		    else
			{ pcour=cir_resul; while(pcour->suc)pcour=pcour->suc;
              pcour->suc=alloue_circuit(49);
		      if(pcour->suc==NULL)pb_allocation(49); 
		      copie_cir(pct,pcour->suc);
              if(option_calcul_z) calculz(pcont->debut_support,pcour->suc);
            }
	        pcisuc=NULL;
		  }
                  
          else if(ind==2)
		   { if(cir_resul==NULL)
				{ cir_resul=alloue_circuit(56);
				  if(cir_resul==NULL)pb_allocation(56);
		           copie_cir(pci,cir_resul);
				}
		     else
				{ pcour=cir_resul; while(pcour->suc)pcour=pcour->suc;
                  pcour->suc=alloue_circuit(56);
				  if(pcour->suc==NULL)pb_allocation(56);
                  copie_cir(pci,pcour->suc);
	            }
	         if(pcipred==NULL)cir_traite=pcisuc;
			 else pcipred->suc=pcisuc;
			}

           else /* insere le resultat dans la chaine */
		   { if(cir_resul==NULL)cir_resul=pres;
		     else
				{ pcour=cir_resul; while(pcour->suc)pcour=pcour->suc;
                  pcour->suc=pres;
				}
			}
	   pci=pcisuc;
	  }
     pct=pct->suc;
    }
// MODIF mars 2006
//   pc=pc->suc;
//  }
// Fin MODIF mars 2006

  /* desalloue le circuit de travail */

  desalloue_chaine_circuit(cir_trav,49);

   /* ajoute les trous calcules au contour resultat */
 pcour=resul_debut->debut_interieur;
 if(cir_resul)
	{if(pcour)
  	  {while(pcour->suc)pcour=pcour->suc;
   	   pcour->suc=cir_resul;
  	  }
 	 else resul_debut->debut_interieur=cir_resul;
	}
}
/*--------------------------------------------------------------------------------*/
int traite_etape2_union(pcir)
struct circuit **pcir;
{struct contour *pc,*res;
 struct circuit *pci;
 int ind;
  pci=NULL;
  res=alloue_contour(45); if(res==NULL)pb_allocation(45);
  ind=masque(res,3);
  if(ind!=0)desalloue_chaine_contour(res);
  if(ind==1) /* le support est inclus dans un trou */
    {*pcir=NULL; return(ind);}
  if(res->debut_support!=NULL) 
       {pc=res;
	while(pc)
	 {if(pci==NULL)pci=pc->debut_support;
	  else {pci->suc=pc->debut_support;pci=pci->suc;}
          pc=pc->suc;
	 }
	libere_contour(res,11);
       }
  *pcir=pci;
  return(ind);
}
/*------------------------------------------------------------------------------------*/
int copie_support_contour(pcont,res)
struct contour *pcont,*res;
{
    res->debut_support=alloue_circuit(289); if(res->debut_support==NULL)pb_allocation(289);
   copie_cir(pcont->debut_support,res->debut_support);
}

