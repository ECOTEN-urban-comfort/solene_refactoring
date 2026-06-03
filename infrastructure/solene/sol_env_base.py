def creer_param_simulation_ts(self, HR=True, T_init=None, hc=None):
    self.creer_descripteur("emissivite")
    self.creer_T_int()
    self.creer_fichier_paroi()
    self.creer_numero_paroi()
    self.creer_meteo_sol()
    self.creer_option_resul()

    if self.profile.enable_water_descriptors:
        self.creer_h_eau_sol()
        self.creer_dt_arrosage_sol()
        self.creer_h_eau_sol_init()

    if T_init:
        self.creer_T_init(T_init)

    if hc:
        self.creer_descripteur_constant("hc", hc)

    if HR:
        self.creer_evaporation()
        self.creer_descripteur_constant("HR", 0)