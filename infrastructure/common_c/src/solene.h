
/***************************************************************/

#include<stdio.h>
#include<math.h>
#include<malloc.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "solenetmp.h"

#define LINUX

int traite_face1;
int traite_face2; /* pour Impression faces en cause si PB */

struct face_fichier {int debut,fin;}*fface,*hfface;

struct modelisation_face { int nofac_fichier;
						   double vnorm[4];
						   double fen[6];
                           int inverse;
                           struct contour *debut_projete  ;
	                   struct contour *debut_dessin;
	                 }*face;
int nbfac,nbfacori; /* nombre de faces lues dans le fichier */
int nomax,nbcontour;
int nb_etat;
struct contour{ int *etat;
	        struct circuit *debut_support;
		struct circuit *debut_interieur;
		struct contour *suc;
	      };

struct circuit{ int nbp;
                  int statut;  /* 0:face plane 1:face non plane 2:ligne 2D  3:ligne 3D */
		  float transp;
		  float visible;
                  double fen[6];
                  double vnorm[4]; /* A VOIR pourrait etre supprime si on avait le no de la face */
		  double *x;	            
		  double *y;
	          double *z;
		  struct circuit *suc;
		 }cir0,cir1; /* cir0:circuit situe derriere */
                             /* cir1:circuit situe devant   */
struct point{ double x;
	      double y;
              double z;
              int statut;
              struct point *suc;
	    };


struct fenetre {double xmin;
                double xmax;
                double ymin;
                double ymax;
               }fen[2];

struct vue {
	    double x;
	    double y;
	    double z;
	    double xo;
	    double yo;
	    double zo;
	    double tran[9];
	   }obs;

double fen_aff[8];

double save_obs_x,save_obs_y,save_obs_z;
double save_obs_xo,save_obs_yo,save_obs_zo;
double save_obs_tran[9];
double save_fen_aff[8];

struct observateur
         {double x;
	  double y;
          double z;
          char libelle[25];
          struct observateur *suc;
	 }*debut_obser,*obser;

struct valeur_face
	 {int nofac;
	  double val;
	 };

int nbcompar;
int nb_obser;
char nom_enreg[25],nom_enreg2d[25],nom_obs[25];
char imp[256],imp1[256],imp2[256],imp3[256],imp4[256];
int dessin; /* =0 si acun traitement realise */

int singularite,non_singularite;
int pb_masque;
double colle_max,pi;
int normale_orientee,traitement_faces_cachees;
double pax,pbx,pay,pby;

/* pour fichier normalise */
#define DISCOL .00001
#define DISDE .000001
#define SURFMIN .00001

#define ANGLE_MIN .001

/*
#define NBCONTMAX  5000
#define NBPTCIRMAX  20000
#define NBPOINT 1000
#define NBCIRMAX 5000
#define NBFACEMAX 5000
#define NBPOLYMAX 4000
#define NBLISTEPOINT 1000
#define NBINTERS 500

int pteur_face[NBFACEMAX];
int pteur_circuit[NBCIRMAX];
int pteur_circuit_point[NBPTCIRMAX];
int pteur_point[NBPOINT];
int pteur_contour[NBCONTMAX];
int pteur_poly[NBPOLYMAX];
int pteur_liste_point[NBLISTEPOINT];
int pteur_inters[NBINTERS];
*/

#define NBSOURCEMAX 20

/*----------------------------*/
/*       LIB_SOLENE_94.C      */
/*----------------------------*/

int		compose_nom_complet();
int		creer_OK_Solene();
FILE*   createFileTmpSolene(char* tmpFileName, char* mode); // SII DFA 18-04-2006
void	cleanFileTmpSolene(char *filename); // SII DFA 24-4-2006
int		existOkSolene();
void	cleanOkSolene();

/*----------------------------*/
/*       PERS_UTIL.C          */
/*----------------------------*/

int		face_dans_vision();
int		visible_pers();

/*----------------------------*/
/*       SOLUTILE.C           */
/*----------------------------*/

double	*alloue_double();
float	*alloue_float();
int		*alloue_int();

int		coplanaire();
int		desalloue_face();
int		desalloue_fface();
int		desalloue_contour_face();
int		desalloue_double();
int		desalloue_float();
int		desalloue_int();

int		lit_en_tete();

int		traite_coplanerite();

struct liste_point *alloue_liste_point();
struct inters *alloue_inters();

struct modelisation_face *alloue_face();

int pb_allocation();
int verif_contour();
int verif_circuit();
int verif_point();
int verif_circuit_point();
int alloue_point_circuit();
struct circuit *alloue_circuit();
struct point *alloue_point();
struct contour *alloue_contour();
int desalloue_point();
int desalloue_contour();
int desalloue_chaine_contour();
int desalloue_circuit();
int desalloue_point_circuit();
struct valeur_face *alloue_valeur();
int desalloue_valeur();

void	lect_fic_val();
int		lit_fic_etat();
int		lit_fic_valeur();
int		nb_contour_face();
int		nbcontours_total();

int affecte_fic_valeur();
int output_valeur_sur_fichier();
int lit_fic_cir3d();
int output_face_sur_fichier();
int output_circuit_sur_fichier();
int ecrit_circuit();

struct contour *copie_contour();
int copie_cir();
int cirsol();
int copie_projete_dessin();
int copie_face();
int copie_face_projete();
int egal_face();
int concatene_face();
int concatene_fface();

int init_cir();
int init_fenetre_affichage();
int init_verif_pteur_solene();
int init_res();

int lit_intervalle();
int test_egalite_de_2_chaines();
int sauve_par_aff();
int restore_par_aff();
int z_min_max_face();
int nb_trou_contour();
double calcul_zplan();
int calculz();
int calcul_d_du_plan();
int place_eq_plan_au_cir();

void read_ligne_vis();
int liste_face();
int liste_struct_contour_ecran();
int liste_circuit_ecran();
int liste_pointeur_circuit();
int liste_pointeur_contour();
int liste_pointeur_circuit_point();
int liste_pointeur_point();

int fentra();
int cal_fen_aff();
int ajuste_fen_affichage();
int normalise_face();
int normalise_cir();
int normalise_point();
int denormalise_point();
int normalise_pts();
int denormalise_face();
int denormalise_circuit();
int fen_arete();
int recoup();
int recoup_circuit();
int fenetre_circuit();
int fen_face();
int tranfo();
int trancir();
int trancir_inverse();
int translation();
int translation_inverse();
int tranp();
int tranp_inverse();
int visible_axono();
int tran_face();
int tran_face_inverse();
int tran_normale_inverse();
int tran_normale();
int test_si_inv_sens();
int inverse_face();
int trancirsol();
int pers_conic_face();
int proj_sphere_face();
int proj_nusselt();
int def_ellipse();
int poly_arc_ellipse();
double ang_ellipse();


int desalloue_contour();
int desalloue_chaine_contour();
int desalloue_circuit();
int desalloue_point_circuit();
int desalloue_inter();
int desalloue_sing();
int inpoly();
int recoup();
int inter();
double dmin();
double dmax();
int sup_sing();
int colpt();
int supt();
int copie_cir();
int init_cir();

int fen_arete();
int inpoly();
int recoup();
int recoup_circuit();
int inter();
double dmax();
double dmin();
int  colpt();
int supt_point();
int supt();
int calculz_inter();
int fenetre_circuit();
int tstin();
int sensz();
int invsens();
double calcul_zplan();
double test_distance();
int airpol();
double surface_point();
double surface();
double cos_angle_vec();
double cos_angle_vec_3D();
double perimetre_poly();
double surface_face();
double cal_surf_face();
int angle_pol();
int liste_struct_inter();
int liste_struct_inter_ecran();
int liste_struct_sing();
int liste_struct_contour_ecran();
int liste_struct_contour_res();
int liste_struct_face_ecran();
int liste_struct_face_res();
int liste_struct_contour();
int liste_circuit();
int liste_circuit_ecran();
int liste_point();
int init_fenetre_affichage();
int fentra();
int cal_fen_aff();
int normale();
int pb_allocation();
int imprime();
int impri1();

struct contour *alloue_contour();
struct point *alloue_point();

int sup_surf_circuit();
int sup_surf();

int traite_coplanerite();

/*---------------------------*/
/*         GRAF.C            */
/*---------------------------*/
int init_graphic();
int affiche_face();
int affiche_contour();
int aff_sphere_face();
int aff_sphere_contour();
int ajou_pts();
int x_affichage();
int y_affichage();
int affiche_un_circuit();
int def_degrade();
int def_fond();
int init_couleur();
int couleur_deg();
int calcul_nocolor();
int def_source();
int aff_cadre();
int pts_cadre();
int affiche_axono();
int affiche_pers();
int affiche_sphere();
/*----------------------------*/
/*   SOLAIRE.C                */
/*----------------------------*/
int info_solaire();
int xyz_soleil();
double angdeg();
double angrad();
int numjour();
double declinaison();
double anglev();
void azihaut();
void compxyz();
double incid();
int vuparsol();
void vcompxyz();
double vincid();
int vvuparsol();
int heure_et_minute_EN_heure_minute();
int heure_et_minute_EN_heure_double();
int heure_et_minute_EN_anglehoraire();
int heure_minute_EN_heure_et_minute();
int heure_minute_EN_heure_double();
int heure_minute_EN_anglehoraire();
int heure_double_EN_heure_et_minute();
int heure_double_EN_heure_minute();
int heure_double_EN_anglehoraire();
int anglehoraire_EN_heure_et_minute();
int anglehoraire_EN_heure_minute();
int anglehoraire_EN_heure_double();
double rii();
double rbsolclair();
double rdsolclair();
double rgsolclair();
double rbsol();
double rgsol();
double rdsol();
double rbplan();
double rdplan();
double rgplan();
double densite_flux_direct();
/*----------------------------*/
/*      FIC_OP_FIC.C          */
/*----------------------------*/
int format_entree_fic_op_fic();
int traite_fic_op_fic();
int compare();
int traite_fic_moins();
int traite_fic_inter();
int traite_fic_union();
/*----------------------------*/
/*      FACE_OP_FACE.C        */
/*----------------------------*/
int face1_moins_face2();
int etape1_moins();
int etape2_moins();

int etape3_moins();
int etape4_moins();
int face1_inter_face2();
int etape1_inter();
int etape3_inter();
int etape4_inter();
int cir_union_cir();
struct circuit *support_trou();
int affecte_trou();
struct circuit *copie_trou_circuits();
int face1_union_face2();
struct contour *union_chaine_contour();
struct contour *chaine_union_chaine();
int contour_union_contour();
int etape1_union();
int etape3_union();
int traite_etape3_union();
int englobant();
int etape2_union();
int traite_etape2_union();
int copie_support_contour();
/*----------------------------*/
/*         TRAITE.C           */     
/*----------------------------*/
int traite_moins();
int devder();
int traite_union();
int traite_inter();
/*----------------------------*/
/*    POLY_OP_POLY.C          */
/*----------------------------*/
int masque();
int liste_lcir_ecran();
int intcor();
int ajout_point();
int statut_point();
int intersecte();
int verif_intersecte();
int liste_intersecte();
struct liste_point *test_sup_inter();
int sup_inter();
int lit_intersecte();
struct liste_point *lcopie();
struct liste_point *desalloue_lcir();
struct spol *copie_lcir_spol();
int nbp_lcir();
int z_cir1();
struct spol *alloue_poly();
int desalloue_poly();
int initialise_statut_point();
int typage_des_aretes_communes();
int typage_des_aretes_int_ext();
char calcul_statut();
struct circuit *copy_poly_cir();
int quelle_methode();
int poly_inter_poly();
int construit_poly_inter();
int poly_moins_poly();
int construit_poly_moins();
int poly_union_poly();
int construit_poly_union();
int construit_poly_operation_nonsingul();
int parcours_polygone_nonsingul();
int poly_disjoint_inclus_egal();
int cherche_segment_du_type();
int cherche_no_inter();
int test_fin_parcours();
int cherche_xy_ds_poly();
int parcours_arete_du_type();
int place_point_ds_poly_resultat();
int epure_polygone_resultat();
int ajoute_cir_pcont();
int recherche_pt_commun_ds_poly();
int constitue_poly_decouple();
int imprime_poly_decouple();
int renseigne();
int intcor1();
int ajout_point1();
int statut_point1();
int intersecte1();
int verif_intersecte1();
struct liste_point *test_sup_inter1();
int sup_inter1();
int lit_intersecte1();
int liste_poly();

/*----------------------------*/
/*       GEOMUTILE.C          */
/*----------------------------*/
int		airpol();
void	centre_de_gravite();
double	cos_angle_vec();
int		inter_droite_plan();
int		inter();
int		inter_plan_plan();
int		point_dans_face();
double	surface();
double	surface_point();
double perimetre_poly();
double surface_face();
double cal_surf_face();
int angle_pol();
double dmax();
double dmin();
int normale_avec_3pts();
int normale();
double test_distance();
int supt_point();
int sup_surf_circuit();
int sup_surf();
int test_point_interieur();
int supt();
int  colpt();
int inpoly();
int sens_face();
int sensz();
int invsens();
int tstin();
int test_point_milieu();

/*----------------------------*/
/*    DECOUPE_FACE_PLAN.C     */
/*----------------------------*/
int decoupe_face_plan();
int test_face_plan();
int test_fenetre();
int coupe_fen();
int test_face1_dessus();
int face_coupe();


/************* DECLARATION POUR LA VERSION DES *************/
/*******************DECALAGES*******************************/

struct circinter{ int nbi;
                  struct liste_inter *debut;
                  struct liste_inter *fin;
                } pintor,pinter;

struct liste_inter{int statut;		  
		   double x;  
                   double y;
                   double z;
                   int ncir0;
                   int ncir1; 
                   struct liste_inter *nori;
                   struct liste_inter *suc;
                   struct liste_inter *pred;
                  };

struct singularite{int nbsing;
                   struct  liste_sing *nosing_debut;
                  } sing; 

struct liste_sing{int nosing[2];
	      	  int statut;
                  struct liste_sing *suc;
                 };   

/*--------------------------*/
/*       MASQUE.C           */
/*--------------------------*/

int masque();
int cherche_englobant();
int copie_inter_orig();
int intcor();
int ajout_sing();
struct liste_sing *verif_sing();
int ajout_sing2();
int sup_sing();
int ajouin();
int verif_inter();
int intcir();
int ajout_pinter();
int tri_inter_cir0();
double inverse_inter_cir0();
int tri_inter_cir1();
double inverse_inter_cir1();
int notpol();
int andpol();
int orpol();
int init_traitement();
int fin_traitement();
int ajout_cir0_croissant();
int ajout_cir1_decroissant();
int ajout_point_decroissant();
int ajout_cir1_croissant();
int ajout_point_croissant();
int decal();
int decal_point();

FILE *fp,*hfp;
FILE *ftest,*fres,*fobs,*fpt2,*fpt3,*fpt4;

#define TAILLE_ECRAN 840
double dim_aff,xor_aff,yor_aff;

FILE *pf_param;
int feuille,nbl,nbc,nb_cadre;
float dcadre,ecart_x,ecart_y,largeur,hauteur,marge_x,marge_y;
double *xcad,*ycad;
int nodessin,dessinok,nofeuille;
char nom_des[20],fourtou[50];
FILE *pf_hpgl;

/*Pixrect *tram[50];*/


/************* DECLARATION POUR LA VERSION DES *************/
/*******************SINGULARITES****************************/

struct inters{ int noa1,noa2;
	       struct inters *suc;
             }*debut_inter;

struct liste_point{int statut,noinit;
		   double x,y,z;
                   struct liste_point *suc;
		  }*lcir0,*lcir1;

struct spol { int nbp;
              char *statut_point;  /* Quelconque,Intersection,Commun */
              char *statut_arete;  /* Interieur,Exterieur,Commun */
              char *traite_arete;  /* Traite arete si Oui */
              double *x,*y,*z;
            } ;



/*--------------------------*/
/*       CURTIS.C           */
/*--------------------------*/

int masque();
int intcor();
int liste_lcir();
int liste_lcir_ecran();
int ajout_point();
int statut_point();
int intersecte();
int verif_intersecte();
int liste_intersecte();
int sup_inter();
int lit_intersecte();
struct liste_point *lcopie();
/*----------------------------*/
/*       SINGUL.C             */
/*----------------------------*/
float xc;   /* pour attribution auto des coords au poly */

struct spol *copie_lcir_spol();
int nbp_lcir();
int z_cir1();

struct spol *alloue_poly();
int initialise_statut_point();

int typage_des_points();
int typage_des_aretes_communes();
int typage_des_aretes_int_ext();
char calcul_statut();
struct circuit *copy_poly_cir();

int poly_inter_poly();
int construit_poly_inter();
int poly_moins_poly();
int construit_poly_moins();
int poly_union_poly();
int construit_poly_union();

int poly_disjoint_inclus_egal();
int cherche_segment_du_type();
int cherche_xy_ds_poly();
int parcours_arete_du_type();
int place_point_ds_poly_resultat();

int epure_polygone_resultat();
int recherche_pt_commun_ds_poly();
int constitue_poly_decouple();
int imprime_poly_decouple();
/**** pour calcul du temps ecoule *****/
double seconde();
char *tecoule();
double tdebut();
FILE *tps;
char temps[50];

int couleur_proc();
int fond();
int source();
int degrade();
int ep_trait();

/*____________________________________________________________________*/
/* gestion de la COULEUR */ 

#define NBCOUL 128
int nbcoul,coul_debut;
int nbtrame,trame_seuil[100];
unsigned char rouge[NBCOUL];
unsigned char vert[NBCOUL];
unsigned char bleu[NBCOUL];


unsigned char rouge_deg[2];
unsigned char vert_deg[2];
unsigned char bleu_deg[2];
double xsource[NBSOURCEMAX],ysource[NBSOURCEMAX],zsource[NBSOURCEMAX];
int nbsource;

/* struct pr_brush brush;  */      /*epaisseur du trait d'affichage */
