/*

cc -c solaire.c -lm 

*/
/*_________________________________________________________________________*/
/* TRANSFORMATIONS ANGULAIRES                                              */
/* TRANSFORMATIONS HORAIRES                                                */
/* CALCUL RAYONNEMENT                                                      */
/*_________________________________________________________________________*/

#include<solene.h>
#define PI 4.*atan(1.);

/*_________________________________________________________________________*/
int info_solaire(latitude,nojour,nomois,hh,minute,xyzsol,a,h)
double latitude,*xyzsol,*h,*a;
int nojour,nomois,hh,minute;
{                    /* latitude donnee en Degre */
 int i,j,k,b1,g1;
 double declin,azi,haut,heure_lever,ang_hor,hm;

    latitude=angrad(latitude);
    i=numjour(nojour,nomois);
    i=(i-1)%365+1;          
    declin=declinaison(i);  
    
    heure_et_minute_EN_anglehoraire(hh,minute,&ang_hor);
    azihaut(latitude,declin,ang_hor,&azi,&haut);
    vcompxyz(azi,haut,xyzsol);
    *a=azi; *h=haut;

}
/*_________________________________________________________________________*/
int xyz_soleil(latitude,declin,hh,minute,xyz)
double latitude,declin;
int hh,minute;
double *xyz;
{   double azi,haut,ang_hor;
             /* latitude et declin en radian */
             /* position du soleil ds xyz */
      heure_et_minute_EN_anglehoraire(hh,minute,&ang_hor);
      azihaut(latitude,declin,ang_hor,&azi,&haut);
      vcompxyz(azi,haut,xyz);
}

/***************************************************************************/
/*                                                                         */
/* TRANSFORMATIONS ANGULAIRES                                              */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Transforme Angle Radian en Degres                                       */
/***************************************************************************/
double angdeg(ang)
double ang;
{double pi;
 pi=PI;
 ang=ang*180./pi;
 return(ang);
}

/***************************************************************************/
/* Transforme Angle Degres en Radian                                       */
/***************************************************************************/
double angrad(ang)
double ang;
{double pi;
 pi=PI;
 ang=ang*pi/180.;
 return(ang);
}

/***************************************************************************/
/* Numero du Jour dans Annee a partir de Date(jour,mois)                   */
/***************************************************************************/
int numjour(jour,mois)
int jour,mois;
{
 int mois2;
 mois--; mois2=mois/2; jour=jour+(mois-mois2)*31+mois2*30;
 if(mois>=2)jour=jour-2;
 if(mois==8||mois==10)jour++;
 return(jour);
}

/***************************************************************************/
/* Declinaison du Jour dans Annee                                          */
/* sin(dec)=0.4*sin(.017*(j-80)) ; RETURN VALUE declinaison en radian      */
/***************************************************************************/
double declinaison(nojour)
int nojour;
{
 double ddec;
 ddec=0.017*(nojour-80); ddec=0.4*sin(ddec); ddec=asin(ddec);
 return(ddec);
}

/***************************************************************************/
/* Angle Horaire au Lever pour Declinaison et Latitude <|66deg33min|       */
/* cos(ah0)=-tan(lat)*tan(dec)                                             */
/***************************************************************************/
double anglev(dec,lat)
double dec,lat;
{
 double alev;
 alev=-acos(-tan(dec)*tan(lat));
 return(alev);
}

/***************************************************************************/
/* Calcule Azimut et Hauteur du soleil pour declinaison                    */
/* latitude et angle horaire                                               */
/* sin(haut)=sin(lat)*sin(dec)+cos(lat)*cos(dec)*cos(ah)                   */
/* sin(azi)=cos(dec)*sin(ah)/cos(haut)                                     */
/* cos(azi)*cos(haut)=sin(lat)*cos(dec)*cos(ah)-cos(lat)*sin(dec)          */
/***************************************************************************/
void azihaut(lat,dec,ah,azi,haut)
double lat,dec,ah,*azi,*haut;
{double sinauh,sinazi,cosazi,pi;
 pi=PI;
 *azi=0.; *haut=pi/2.;
      sinauh=sin((double)lat)*sin((double)dec);
      sinauh=sinauh+cos((double)lat)*cos((double)dec)*cos((double)ah);
 if(fabs(sinauh)>=1.) return;
      *haut=(double)asin(sinauh);
      sinazi=cos((double)dec)*sin((double)ah)/cos((double)*haut);
 if(fabs(sinazi)<1.)
      {
      *azi=(double)asin(sinazi);
      cosazi=sin((double)lat)*cos((double)dec)*cos((double)ah);
      cosazi=(cosazi-cos((double)lat)*sin((double)dec))/cos((double)*haut);
      if(cosazi<=0.) *azi=pi-*azi;
      if(*azi>pi) *azi=*azi-2.*pi;
      }
 else { *azi=pi/2. ; if(ah<0) *azi=-*azi; }
}

/***************************************************************************/
/* Composantes X,Y,Z du Vecteur D‚fini par Azimut et Hauteur               */
/***************************************************************************/
void compxyz(azi,haut,x,y,z)
double azi,haut,*x,*y,*z;
{double pi;
 pi=PI;
 if(fabs((double)haut) < pi/2.)
  { *x=-sin((double)azi); *y=-cos((double)azi); *z=tan((double)haut); }
 else
  { *x=0.; *y=0.; *z=1. ; if(haut<0) *z=-1; }
}

/***************************************************************************/
/* Angle entre 2 vecteurs de composantes x,y,z et xs,ys,zs                 */
/* return value =cos de l'angle                                            */
/***************************************************************************/
double incid(x,y,z,xs,ys,zs,ang)
double x,y,z,xs,ys,zs,*ang;
{double d,pi;
 pi=PI;
 d=sqrt(x*x+y*y+z*z)*sqrt(xs*xs+ys*ys+zs*zs);
 d=(x*xs+y*ys+z*zs)/d;
 if(d>1)  { *ang=0; d=1.; return(d); }
 if(d<-1) { *ang=pi; d=-1.; return(d); }
 *ang=acos(d); return(d);
}

/***************************************************************************/
/* Test SI Face definie par normale de composantes x,y,z  est Vue          */
/* par le Soleil de composantes xs,ys,zs ;                                 */
/* return value = 1 si vue (0 si non ; soleil rasant=0)                    */
/***************************************************************************/
int vuparsol(x,y,z,xs,ys,zs)
double x,y,z,xs,ys,zs;
{
 double d;
 int i;
 d=(x*xs+y*ys+z*zs); i=0; if(d>0) i=1;
 return(i);
}

/***************************************************************************/
/* Composantes c(X,Y,Z) du Vecteur Defini par Azimut et Hauteur            */
/***************************************************************************/
void vcompxyz(azi,haut,c)
double azi,haut,*c;
{double pi,lgv,vx,vy,vz;
 pi=PI;
 if(fabs((double)haut) < pi/2.)
 { vx=-sin((double)azi); vy=-cos((double)azi); vz=tan((double)haut);
   lgv=sqrt(vx*vx+vy*vy+vz*vz);
   *c=vx/lgv; *(c+1)=vy/lgv; *(c+2)=vz/lgv;
 }
 else
 { *c=0.; *(c+1)=0.; *(c+2)=1. ; if(haut<0) *(c+2)=-1; }
}

/***************************************************************************/
/* Angle entre 2 vecteurs de composantes c(x,y,z) et cs(xs,ys,zs)          */
/* return value =cos de l'angle                                            */
/***************************************************************************/
double vincid(c,cs,ang)
double *c,*cs,*ang;
{
 double rang;
 rang=incid(*c,*(c+1),*(c+2),*cs,*(cs+1),*(cs+2),ang);
 return(rang);
}

/***************************************************************************/
/* Test SI Face definie par normale de composantes c(x,y,z) est Vue        */
/* par le Soleil de composantes cs(xs,ys,zs);                              */
/* return value = 1 si vue (0 si non ; soleil rasant=0)                    */
/***************************************************************************/
int vvuparsol(c,cs)
double *c,*cs;
{
 int i;
 i=vuparsol(*c,*(c+1),*(c+2),*cs,*(cs+1),*(cs+2));
 return(i);
}

/***************************************************************************/
/*                                                                         */
/* TRANSFORMATIONS HORAIRES                                                */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*  heure_et_minute_EN_heure_minute   ( heure minute *heure.minute )       */
/***************************************************************************/ 
int heure_et_minute_EN_heure_minute(heure,minute,hm)
int heure,minute;
double *hm;
{ 
   *hm=heure+minute/100.; 
} 
/***************************************************************************/
/*  heure_et_minute_EN_heure_double   ( heure minute *heure.double )       */
/***************************************************************************/
int heure_et_minute_EN_heure_double(heure,minute,hd)
int heure,minute;
double *hd;
{ 
   *hd=heure+minute/60.; 
}
/***************************************************************************/
/*  heure_et_minute_EN_anglehoraire   ( heure minute *anghradian )         */
/***************************************************************************/
int heure_et_minute_EN_anglehoraire(heure,minute,anghr)
int heure,minute;
double *anghr;
{ 
   double ang;

   ang=heure+minute/60.; 
   ang=(ang-12)*15;
   *anghr=angrad(ang);
}                       
/***************************************************************************/
/*  heure_minute_EN_heure_et_minute   ( heure.minute *heure *minute )      */
/***************************************************************************/
int heure_minute_EN_heure_et_minute(hm,heure,minute)
double hm;
int *heure,*minute;
{ 
    *heure=hm;
	*minute = (hm-(*heure))*100;
	//   Ancienne version : *minute=rint((hm-(*heure))*100);
}
/***************************************************************************/
/*  heure_minute_EN_heure.double   ( heure.minute *heure.double )      */
/***************************************************************************/
int heure_minute_EN_heure_double(hm,hd)
double hm,*hd;
{ 
   int heure,minute;

   heure_minute_EN_heure_et_minute(hm,&heure,&minute);
   heure_et_minute_EN_heure_double(heure,minute,hd);
}
/***************************************************************************/
/*  heure_minute_EN_anglehoraire   ( heure.minute *anghradian )            */
/***************************************************************************/
int heure_minute_EN_anglehoraire(hm,anghr)
double hm,*anghr;
{ 
   int heure,minute;

   heure_minute_EN_heure_et_minute(hm,&heure,&minute);
   heure_et_minute_EN_anglehoraire(heure,minute,anghr);
}
/***************************************************************************/
/*  heure_double_EN_heure_et_minute   ( heure.double  *heure *minute)      */
/***************************************************************************/
int heure_double_EN_heure_et_minute(hd,heure,minute)
double hd;
int *heure,*minute;
{ 
  
	*heure=hd; *minute=(hd-(*heure))*60.;

	// Ancienne version : *heure=hd; *minute=rint((hd-(*heure))*60.);
}
/***************************************************************************/
/*  heure_double_EN_heure_minute   ( heure.double  *heure.minute)          */
/***************************************************************************/
int heure_double_EN_heure_minute(hd,hm)
double hd,*hm;
{ 
   int heure,minute;

   heure_double_EN_heure_et_minute(hd,&heure,&minute);
   heure_et_minute_EN_heure_minute(heure,minute,hm);
}
/***************************************************************************/
/*  heure_double_EN_anglehoraire   ( heure.double  *anghradian  )          */
/***************************************************************************/
int heure_double_EN_anglehoraire(hd,anghr)
double hd,*anghr;
{ 
   int heure,minute;

   heure_double_EN_heure_et_minute(hd,&heure,&minute);
   heure_et_minute_EN_anglehoraire(heure,minute,anghr);
}
/***************************************************************************/
/*  anglehoraire_EN_heure_et_minute   ( anghradian  *heure *minute )       */
/***************************************************************************/
int anglehoraire_EN_heure_et_minute(anghr,heure,minute)
int *heure,*minute;
double anghr;
{ 
   anghr=12.+angdeg(anghr)/15.; 
   *heure=anghr;
   *minute=(anghr-(*heure))*60.;
   // ancienne version : *minute=rint((anghr-(*heure))*60.);
}
/***************************************************************************/
/*  anglehoraire_EN_heure.minute   ( anghradian  *heure.minute )           */
/***************************************************************************/
int anglehoraire_EN_heure_minute(anghr,hm)
double anghr,*hm;
{ 
   int heure,minute;

   anglehoraire_EN_heure_et_minute(anghr,&heure,&minute);
   heure_et_minute_EN_heure_minute(heure,minute,hm);
}
/***************************************************************************/
/*  anglehoraire_EN_heure.double   ( anghradian  *heure.double )           */
/***************************************************************************/
int anglehoraire_EN_heure_double(anghr,hd)
double anghr,*hd;
{ 
   int heure,minute;

   anglehoraire_EN_heure_et_minute(anghr,&heure,&minute);
   heure_et_minute_EN_heure_double(heure,minute,hd);
}

/***************************************************************************/
/*                                                                         */
/* CALCUL RAYONNEMENT                                                      */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/* Rayonnement normal fonction de Hauteur du soleil et du trouble du ciel  */
/***************************************************************************/
double rii(haut,trb)
double haut;
int trb;
{double rn,pi;
 double x;
 pi=PI;
 x=haut+2.*pi/180.; x=sin(x);
 if(trb==1)     rn=1300.*exp((double)(-1./(6.*x)));       /* ciel tres pur */
 else if(trb==2)rn=1230.*exp((double)(-1./(4.*x)));       /* ciel moyen    */
 else           rn=1200.*exp((double)(-1./(2.5*x)));      /* ciel pollue   */
 return(rn);
}

/***************************************************************************/
/* Rayonnement Direct sur plan Horizontal  instant t  par CIEL CLAIR       */
/* Fonction (hauteursoleil,rayonnement normal)                             */
/***************************************************************************/
double rbsolclair(haut,rn)
double haut,rn;
{
 double dirsol;
 dirsol=rn*sin((double)haut);
 return(dirsol);
}

/***************************************************************************/
/* Rayonnement Diffus sur plan Horizontal  instant t  par CIEL CLAIR       */
/* Fonction (hauteur soleil,trouble)                                       */
/***************************************************************************/
double rdsolclair(haut,trb)
double haut,trb;
{
 double difsol,x;
 x=sin((double)haut); difsol=pow((double)x,(double)0.4);
 if(trb==1)     difsol=87.*difsol;         /* ciel trŠs pur */
 else if(trb==2)difsol=125.*difsol;        /* ciel moyen    */
 else           difsol=187.*difsol;        /* ciel pollu‚   */
 return(difsol);
}

/***************************************************************************/
/* Rayonnement Global sur plan Horizontal  instant t  par CIEL CLAIR       */
/* Fonction (hauteur soleil,trouble)                                       */
/***************************************************************************/
double rgsolclair(haut,trb)
double haut;
int trb;
{
 double rgsol,x;
 x=sin((double)haut); pow((double)x,(double)0.4);
 if(trb==1)     rgsol=1150.*pow((double)x,(double)1.15); /* ciel trŠs pur */
 else if(trb==2)rgsol=1080.*pow((double)x,(double)1.22); /* ciel moyen    */
 else           rgsol=990.*pow((double)x,(double)1.25);  /* ciel pollu‚   */
 return(rgsol);
}

/***************************************************************************/
/* Rayonnement Direct sur plan Horizontal  instant t  Conditions MOYENNES  */
/* Fonction (rayonnement direct sol ciel clair,fraction insolation)        */
/***************************************************************************/
double rbsol(rbsolclair,fi)
double rbsolclair,fi;
{
 double dirsol;
 dirsol=rbsolclair*fi;
 return(dirsol);
}

/***************************************************************************/
/* Rayonnement Global sur plan Horizontal  instant t  Conditions MOYENNES  */
/* Fonction (global sol ciel clair,fraction insolation)                    */
/***************************************************************************/
double rgsol(gloclair,fi)
double gloclair,fi;
{
 double glosol,x;
 if(fi!=1) { x=sqrt((double)(2.*fi+1)); glosol=gloclair*(x-.72); }
 else glosol=gloclair;
 return(glosol);
}

/***************************************************************************/
/* Rayonnement Diffus sur plan Horizontal  instant t  Conditions MOYENNES  */
/* Fonction (direct sol,global sol)                                        */
/***************************************************************************/
double rdsol(dirsol,glosol)
double dirsol,glosol;
{
 double difsol;
 difsol=glosol-dirsol;
 return(difsol);
}

/***************************************************************************/
/* Rayonnement Direct sur PLAN QUELCONQUE  instant t  Conditions MOYENNES  */
/* Fonction (rayonnement normal,cos incidence,fraction insolation)         */
/***************************************************************************/
double rbplan(rn,cai,fi)
double rn,cai,fi;
{
 double dirplan;
 dirplan=rn*cai*fi;
 return(dirplan);
}

/***************************************************************************/
/* Rayonnement Diffus sur PLAN QUELCONQUE  instant t  Conditions MOYENNES  */
/* Fonction (diffus sol,global sol,albedo,inclinaison normale face radian) */
/***************************************************************************/
double rdplan(difsol,glosol,albedo,incl)
double difsol,glosol,albedo,incl;
{double difplan,pi;
 double vc;
 pi=PI;
 if(incl>=0) { if(incl>=pi/2.)difplan=difsol;
	       else { vc=pi/2.-incl; vc=cos(vc);
		      difplan=.5*difsol*(1.+vc)+.5*albedo*glosol*(1.-vc);
		    }
	     }
 else        { if(incl<=-pi/2.)difplan=albedo*glosol;
	       else { vc=pi/2+incl; vc=cos(vc);
		      difplan=.5*difsol*(1.-vc)+.5*albedo*glosol*(1.+vc);
		    }
	     }
 return(difplan);
}

/***************************************************************************/
/* Rayonnement Global sur PLAN QUELCONQUE  instant t  Conditions MOYENNES  */
/* Fonction (direct plan,diffus plan)                                      */
/***************************************************************************/
double rgplan(dirplan,difplan)
double dirplan,difplan;
{
 double gloplan;
 gloplan=dirplan+difplan;
 return(gloplan);
}

/***************************************************************************/
/* Densite Flux Direct sur PLAN QUELCONQUE  instant t  Conditions MOYENNES */
/* Fonction (compos. soleil,compos. plan recepteur,trouble,f insolation)   */
/***************************************************************************/
double densite_flux_direct(vsol,h,vnorm,trb,fi)
double *vsol,h,*vnorm,fi;
int trb;
{   /* si vnorm=vsol c'est le flux normal */
    /* FAIRE avant appel :info_sol(lati,nojour,nomois,hh,minute,xyzsol,&a,&h)*/
 double dirplan,ang;

 dirplan=rii(h,trb)*vincid(vsol,vnorm,&ang)*fi;
 return(dirplan);

}
