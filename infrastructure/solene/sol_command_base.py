class SolCommand():
    """
    Classe de simulation solene
    
    les chemins vers les fichiers d'entree et de sortie sont definis par défaut
    """
    def __init__(self,
                 chemin_sim,
                 nom_cas,
                 profile,
                 n_ciel = 3):

        self.profile = profile
        self.liste_carac = list(profile.carac_face + profile.carac_triangle)
        self.liste_variables = list(
            profile.variables_clo + profile.variables_glo + profile.variables_transient
        )
        self.liste_variables_confort = list(profile.variables_comfort)
        self.pas_de_temps = profile.default_time_step_s