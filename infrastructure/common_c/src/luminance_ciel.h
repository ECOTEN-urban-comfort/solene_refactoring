
// luminance_ciel.h
// Protype des fonctions

void affiche_soleil(double latit,	//latitude en degré
					double longit,	//longitude en degré
					int jour,		//no du jour
					int mois,		//no du mois
					int jd,			//no du jour dans l'année
					int h_deb,		//heure entiere
					int m_deb,		//minute
					double h,		//hauteur du soleil
					double sd,		// 
					double st,		// 
					double epsilon,	//epsilon du ciel
					double delta,	//delta du ciel
					double lzenit,	//luminance au zenith
					double De,		//
					double Se,		//
					double Dv,		//
					double Sv);		//
double airmass (double h );// hauteur du soleil en radian
double alt ( double sd, double st,  double latit);         
double azim (double sd, double st,  double latit);        
double declin (int jd);
double effic_lum_diffus(double delta , double Z , double w , int interval);
double effic_lum_normal(double delta , double Z , double w , int interval);
double exentr (int jd );
int jdate (int mois ,int jour );
void lect_coef_per(char *filename , double * per_ );
void luminance (double xg , double yg , double h , double az ,double *lumin, double *integr );
void usage_ciel();






