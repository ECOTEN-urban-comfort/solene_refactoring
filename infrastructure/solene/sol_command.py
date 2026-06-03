import os
import subprocess
import time

from infrastructure.geometry.legacy.utils import Chrono, ecrire_fichier
from infrastructure.geometry.legacy.solFile import *
from infrastructure.solene.profiles.surface_model_profile import SurfaceModelProfile

JOIN = os.path.join

COM_ENER_BAT = "simulation_Ts_EnergieBat_laurent"
COM_ENERGIE_BAT_VEG = "simulation_Ts_EnergieBat_VEG"
COM_ENERGIE_BAT_NEW = (
    "/home/laurent/solenetb/src/trunk/"
    "solene.thermo-radiatif.core/exe/simulation_Ts_EnergieBat.exe"
)

LISTE_OPTION = """Tse\t1
Tn1\t0
Tn2\t0
Tsi1\t0
Tsi2\t0
GLO_Ciel_Emis\t0
GLO_Ciel_Recu\t0
GLO_Ciel_Net \t1
GLO_Scene_Emis\t0
GLO_Scene_Recu\t0
GLO_Scene_Net\t0
GLO_Total_Emis\t0
GLO_Total_Recu\t0
GLO_Total_Net\t1
Conv\t\t1
Sol_Ciel_Reflechi \t1
Sol_Net\t\t\t0
Sol_Scene_Apport\t0
"""

NOM_FICHIER_METEO_DIFFUS = "meteo_diffus"
NOM_FICHIER_METEO_DIRECT = "meteo_direct"


class SolCommand:
    """
    Shared refactored SolCommand for all surface models.

    The constructor and the large majority of helper methods are shared.
    Remaining differences between mixture / azam / BB5 are delegated
    through the provided SurfaceModelProfile.
    """

    def __init__(
        self,
        chemin_sim: str,
        nom_cas: str,
        profile: SurfaceModelProfile,
        n_ciel: int = 3,
    ) -> None:
        self.profile = profile
        self.nom_cas = nom_cas
        self.chemin_simul_sol = chemin_sim

        self.chemin_ciel = JOIN(self.chemin_simul_sol, "ciel")
        self.chemin_entree = JOIN(self.chemin_simul_sol, "entree")
        self.chemin_sortie = JOIN(self.chemin_simul_sol, "sortie")

        self.chemin_clo = JOIN(self.chemin_sortie, "CLO")
        self.chemin_transitoire = JOIN(self.chemin_sortie, "transitoire")
        self.chemin_ener = JOIN(self.chemin_sortie, "ener")
        self.chemin_confort = JOIN(self.chemin_sortie, "confort")

        self.scene_cir = JOIN(self.chemin_simul_sol, "scene_triangle")
        self.masque_cir = JOIN(self.chemin_simul_sol, "scene_masque")

        self.sol_confort_cir = JOIN(self.chemin_confort, "sol_confort")
        self.bonhomme_confort_cir = JOIN(self.chemin_confort, "bonhomme_confort")
        self.bonhomme_confort_surface = JOIN(
            self.chemin_confort, "bonhomme_confort_surface"
        )

        self.chemin_masque = JOIN(self.chemin_clo, "masque")
        self.masque_mas = JOIN(self.chemin_masque, "masque")

        self.param = {
            "latitude": 46,
            "longitude": -1,
            "meridien": 0,
            "temp_consigne": 20,
            "taux_occupation": 0.063,
            "puissance_sensible": 0,
            "regime_thermique": 1,
            "saison": 0,
            "angle_vision": 89.9,
            "transparence": True,
            "opt_lumiere": 0,
            "delta_perez": 0.12,
            "epsilon_perez": 6.3,
        }

        # Legacy compatibility attributes
        self.latitude = self.param["latitude"]
        self.longitude = self.param["longitude"]
        self.meridien = self.param["meridien"]
        self.temp_consigne = self.param["temp_consigne"]
        self.taux_occupation = self.param["taux_occupation"]
        self.puissance_sensible = self.param["puissance_sensible"]
        self.regime_thermique = self.param["regime_thermique"]
        self.saison = self.param["saison"]
        self.angle_vision = self.param["angle_vision"]
        self.transparence = self.param["transparence"]
        self.opt_lumiere = self.param["opt_lumiere"]
        self.delta_perez = self.param["delta_perez"]
        self.epsilon_perez = self.param["epsilon_perez"]

        self.liste_jours = [[10, 7, "10:00", "16:00", "01:00"]]

        self.ciel_cir = JOIN(self.chemin_ciel, "ciel")
        self.ciel_angle_solide_val = JOIN(self.chemin_ciel, "angleSolideCiel")
        self.luminance_ciel_val = JOIN(self.chemin_ciel, "luminance")

        self.chemin_meteo_diffus = JOIN(self.chemin_ciel, NOM_FICHIER_METEO_DIFFUS)
        self.chemin_meteo_direct = JOIN(self.chemin_ciel, NOM_FICHIER_METEO_DIRECT)

        self.historique_commande = []
        self.fichier_commande = JOIN(self.chemin_entree, "commande_sim_ts")
        self.liste_ts_sol = []

        self.carac_face = tuple(profile.carac_face)
        self.carac_triangle = tuple(profile.carac_triangle)
        self.variables_clo = tuple(profile.variables_clo)
        self.variables_glo = tuple(profile.variables_glo)
        self.variables_transient = tuple(profile.variables_transient)
        self.variables_comfort = tuple(profile.variables_comfort)

        self.liste_carac = list(self.carac_face + self.carac_triangle)
        self.carac = {}
        self._creer_nom_descripteur()

        self.liste_variables = list(
            self.variables_clo + self.variables_glo + self.variables_transient
        )
        self.liste_variables_confort = list(self.variables_comfort)
        self.var = {}
        self.var_confort = {}
        self._creer_nom_variable()
        self._creer_nom_variable_confort()

        self.facform = JOIN(self.chemin_simul_sol, "facform")

        self.t_meteo = 25.25
        self.pas_de_temps = profile.default_time_step_s
        self.opt_interreflexions = 1
        self.flux_ir = 350.73
        self.option_calc = JOIN(self.chemin_entree, "option_calc")
        self.option_resul = JOIN(self.chemin_entree, "option_resul")
        self.meteo_sol = JOIN(self.chemin_entree, "meteo_sol")
        self.paroi = JOIN(self.chemin_entree, "paroi")
        self.paroi_transitoire = JOIN(self.chemin_entree, "paroi_tr")
        self.sigma = 5.67e-8
        self.coeff_veg = JOIN(self.chemin_entree, "coeff_veg")

        suffix = "%d%.2d%.2d-%.2d%.2d" % time.localtime()[:5]
        self.chemin_fichier_sortie = JOIN(self.chemin_simul_sol, "stdout-" + suffix)
        self.fichier_sortie = open(self.chemin_fichier_sortie, "w")

        self.chemin_poubelle = JOIN("/", "tmp", "poubelle_radiosite")
        self.poubelle = open(self.chemin_poubelle, "w")

        self.bak = []
        self.creer_cas()

    def creer_cas(self) -> None:
        print("\n  CREATION DU CAS SOLENE")

        if not os.path.isdir(self.chemin_simul_sol):
            os.mkdir(self.chemin_simul_sol)
            print("\t creation repertoire -simulSol-")
            print("\t -> ", self.chemin_simul_sol)
        else:
            print("\t -simulSol- existe deja")

        if not os.path.isdir(self.chemin_entree):
            os.mkdir(self.chemin_entree)
            print("\t creation repertoire -entree-")

        if not os.path.isdir(self.chemin_sortie):
            os.mkdir(self.chemin_sortie)
            print("\t creation repertoire -sortie-")

        if not os.path.isdir(self.chemin_clo):
            os.mkdir(self.chemin_clo)
            print("\t creation repertoire -CLO-")

        if not os.path.isdir(self.chemin_transitoire):
            os.mkdir(self.chemin_transitoire)
            print("\t creation repertoire -transitoire-")

        if not os.path.isdir(self.chemin_ener):
            os.mkdir(self.chemin_ener)
            print("\t creation repertoire -ener-")

        if not os.path.isdir(self.chemin_ciel):
            os.mkdir(self.chemin_ciel)
            print("\t creation repertoire -Ciel-")
            self.creer_ciel()

        if not os.path.isdir(self.chemin_masque):
            os.mkdir(self.chemin_masque)
            print("\t creation repertoire -CLO/masque-")
            self.creer_ciel()

    def creer_cas_confort(self) -> None:
        if not os.path.isdir(self.chemin_confort):
            os.mkdir(self.chemin_confort)

    def definir_liste_jours(self, liste_jours):
        """
        definis une nouvelle liste de jours et recalcule les fichiers
        de luminance du ciel dans la foulee
        """
        self.liste_jours = liste_jours
        ok = 1
        for suff in self.liste_ts_sol:
            nom_fichier = '%s_%s.val' % (self.luminance_ciel_val, suff)
            if not os.path.isfile(nom_fichier):
                ok = 0

        if not ok:
            self.calculer_luminance_ciel()
        else:
            print('\tles fichiers luminances existent')

    def definir_meteo_dic(self, liste_meteo):
        """
        Imports the weather from a weather_list
        into a dictionary { suff: weather_list[i] }
        """
        dic_meteo = {}
        
        if len(liste_meteo) == len(self.liste_ts_sol):
            for i, ts_sol in enumerate(self.liste_ts_sol):
                dic_meteo[ts_sol] = liste_meteo[i]
                
        self.dic_meteo = dic_meteo

    def creer_ciel(self, ordre = 3, creation_angle_solide = True):
        """
        création du ciel
        
        appelle la commande externe : 'geode_ciel'
        avec comme argument ordre = 3
        crée les angles solides par défaut
        """
        com = ['geode_ciel',
             self.ciel_cir,
             str(ordre)]
	     
        retcode = subprocess.call(com, 
						stdout = self.fichier_sortie, 
						stderr = self.fichier_sortie)
        self.historique_commande.append(com)

        if creation_angle_solide:
            self.creer_angle_solide()

        return retcode

    def creer_angle_solide(self):
        """
        création des angles solides
        
        appelle la commande externe 'angl_solid'
        """
        com = ['angl_solid',
             self.ciel_cir,
             self.ciel_angle_solide_val]

        retcode = subprocess.call(com,
						stdout = self.fichier_sortie, 
						stderr = self.fichier_sortie)
        self.historique_commande.append(com)
        
        return retcode

    def calculer_surface(self):
        """
        calcul des surfaces de la géométrie
        
        appelle lea commande externe 'surf_cont'
        """
        com = ['surf_cont',
             self.scene_cir,
             self.carac['surface']]
             
        retcode = subprocess.call(com)
        self.historique_commande.append(com)
        
        return retcode
    
    def calculer_luminance_ciel(self, format_num = False):
        """ 
        créations des fichiers de luminance de ciel
        pour les fonctions faisant appel au solaire diffus.

        doivent être renseignés:
             latitude, longitude, type de ciel (epsilon, delta)
             liste des jours

        choix du nommage des fichier
             ajoute _jjmm_hhHmm (défaut)
             ajoute _jjmm(num) si format_num = True

        """
        for i, jour in enumerate(self.liste_jours):
            com = ['luminance_ciel_temps',
                   self.ciel_cir,
                   str(self.param['latitude']),
                   str(self.param['longitude']),
                   self._composerJJMM(i),       # jour (jour/mois)
                   str(jour[2]), # heure debut (hh:mm)
                   str(jour[3]),      # heure fin (hh:mm)
                   str(jour[4]),      # pas (hh:mm)
                   str(self.param['epsilon_perez']),
                   str(self.param['delta_perez']),
                   '2',
                   self._composer_nom(self.luminance_ciel_val,i)]
            
            if format_num: 
                com[0] = 'luminance_ciel'
             
            retcode = subprocess.call(com,
                                    stdout = self.fichier_sortie, 
                                    stderr = self.fichier_sortie)
            self.historique_commande.append(com)


        return retcode

    def calculer_flux_sol_direct(self, 
                                 nb_transmission = -1, 
                                 fichier_meteo = None,
                                 execute = True):
        """ 
        calcul du rayonnement solaire direct

        doivent être renseignés:\n
           ** geometrie scene
           ** géométrie masque
           ** transmittivité
           ** latitude,
           ** liste des jours

        """
        chrono = Chrono('calculer_flux_sol_direct', 'solCommande') 
        if self.param['transparence'] and fichier_meteo == None:
            for i, jour in enumerate(self.liste_jours):
                print('=  calcul jour : %s' % i)
                
                com = ['masques_sol_lum',
                       self.scene_cir,             #
                       self.masque_cir,            #
                       self.carac['transmittance'],    #
                       str(self.param['latitude']),        #
                       self._composerJJMM(i),   # jour (jour/mois)
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       str(self.param['angle_vision']),     # 89.9 par defaut
                       str(nb_transmission),       # fonction (-1 => toutes)
                       self._composer_nom(self.var['flux_sol_direct'], i),     
                       str(self.param['opt_lumiere'])]      # fonction (0 => W/m2) 

                
                retcode = subprocess.call(com,
                                        stdout = self.fichier_sortie, 
                                        stderr = self.fichier_sortie)
                self.historique_commande.append(com)     
       
        if self.param['transparence'] == False:
            print('to do') # To Do!
            
        if fichier_meteo and nb_transmission == 0:
            for i, jour in enumerate(self.liste_jours):
                print('= calcul jour : ', str(i))
                
                com = ['energie_solaire_directe_meteo',
                       self.scene_cir,
                       self._composer_nom(self.masque_mas, jour),
                       self._composerJJMM(jour),
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       self._composer_nom(self.chemin_meteo_direct, i),
                       str(self.param['latitude']),
                       self._composer_nom(self.var['flux_sol_direct'], i)]
                print(com)

               
                retcode = subprocess.call(com,
                                        stdout = self.fichier_sortie, 
                                        stderr = self.fichier_sortie)
                
                self.historique_commande.append(com)       

        if fichier_meteo and nb_transmission == -1:
            for i, jour in enumerate(self.liste_jours):
                print('=  calcul jour : %s' % i)
                
                com = ['masques_sol_lum_meteo',
                       self.scene_cir,             #
                       self.masque_cir,            #
                       self.carac['transmittance'],    #
                       str(self.param['latitude']),        #
                       self._composerJJMM(i),   # jour (jour/mois)
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       str(self.param['angle_vision']),     # 89.9 par defaut
                       str(nb_transmission),       # fonction (-1 => toutes)
                       self._composer_nom(self.chemin_meteo_direct, i),
                       self._composer_nom(self.var['flux_sol_direct'], i),     
                       str(self.param['opt_lumiere'])]      # fonction (0 => W/m2)
                
                if execute:
                    retcode = subprocess.call(com,
                                              stdout = self.fichier_sortie, 
                                              stderr = self.fichier_sortie)
                else:
                    retcode = 0
                self.historique_commande.append(com)

        chrono.fin()
        
        return retcode

    def calculer_flux_sol_diffus(self, 
                                 nb_transmission = -1, 
                                 fichier_meteo = None):

        """
        calcul du rayonnement solaire diffus
        
        appelle la fonction extene 'masques_ciel_lum'
        
        doivent être renseignés
            ** geom scene
            ** geom masque
            ** transmittivite masque
            ** ciel + angles solides
            ** liste jours
        """
        chrono = Chrono('calculer_flux_sol_diffus', 'solCommande') 
        
        if self.param['transparence'] and fichier_meteo == None:
            for i, jour in enumerate(self.liste_jours):
                print('= calcul jour : %s' % i)
                
                com = ['masques_ciel_lum',
                       self.scene_cir,             #
                       self.masque_cir,            #
                       self.carac['transmittance'],    #
                       self.ciel_cir,              #
                       self.ciel_angle_solide_val,   #
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       self._composer_nom(self.luminance_ciel_val, i),
                       str(self.param['angle_vision']),     # 89.9 par defaut
                       str(nb_transmission),       # fonction (-1 => toutes)
                       self._composer_nom(self.var['flux_sol_diffus'], i),
                       str(self.param['opt_lumiere'])]        # fonction (0 => W/m2) 
                

                retcode = subprocess.call(com,
                                        stdout = self.fichier_sortie, 
                                        stderr = self.fichier_sortie)
                self.historique_commande.append(com)
                
        if fichier_meteo and nb_transmission == 0:
            for i, jour in enumerate(self.liste_jours):
                print('= calcul jour : %s' % jour)
                
                com = ['energie_solaire_diffuse_meteo',
                       self.scene_cir,             #
                       self.masque_cir,            #
                       self.ciel_cir,              #
                       self.ciel_angle_solide_val,   #
                       self._composer_nom(self.luminance_ciel_val, i),
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       str(self.param['angle_vision']),     # 89.9 par defaut
                       self._composer_nom(self.chemin_meteo_diffus, i),
                       self._composer_nom(self.var['flux_sol_diffus'], i)] 
                       # fonction (0 => W/m2) 
                         
                retcode = subprocess.call(com,
                                        stdout = self.fichier_sortie, 
                                        stderr = self.fichier_sortie)
                self.historique_commande.append(com)

        if fichier_meteo and nb_transmission == -1:
            for i, jour in enumerate(self.liste_jours):
                print('= calcul jour : %s' % i)
                
                com = ['masques_ciel_lum_meteo',
                       self.scene_cir,             #
                       self.masque_cir,            #
                       self.carac['transmittance'],    #
                       self.ciel_cir,              #
                       self.ciel_angle_solide_val,   #
                       jour[2],  # heure debut (hh:mm)
                       jour[3],  # heure fin (hh:mm)
                       jour[4],  # pas (hh:mm)
                       self._composer_nom(self.luminance_ciel_val, i),
                       str(self.param['angle_vision']),     # 89.9 par defaut
                       str(nb_transmission),       # fonction (-1 => toutes)
                       self._composer_nom(self.chemin_meteo_diffus, i),
                       self._composer_nom(self.var['flux_sol_diffus'],i)] 
                       # fonction (0 => W/m2) 

                retcode = subprocess.call(com,
                                        stdout = self.fichier_sortie, 
                                        stderr = self.fichier_sortie)
                self.historique_commande.append(com)
                self.bak.append(com)
 
        chrono.fin()
 
        return retcode

    def calculer_flux_sol_total(self, geo = None):
        """
        realise la somme des flux directs et diffus pour obtenir le flux
        total. Utilise val_op_val
        """
        for suffix in self.liste_ts_sol:
            if geo:
                flux_sol_direct = read_val(self.var['flux_sol_direct'] + '_' + suffix, geom=geo)
                flux_sol_diffus = read_val(self.var['flux_sol_diffus'] + '_' + suffix, geom=geo)
                flux_sol_total = flux_sol_direct+ flux_sol_diffus
                write_val(self.var['flux_sol_total'] + '_' + suffix, geo, flux_sol_total)
                retcode = 0
            else:
                com = ['val_op_val',
                       self.var['flux_sol_direct'] + '_' + suffix,
                       '+',
                       self.var['flux_sol_diffus'] + '_' + suffix,
                       self.var['flux_sol_total'] + '_' + suffix]
                retcode = subprocess.call(com,
                                          stdout = self.fichier_sortie, 
                                          stderr = self.fichier_sortie)
                self.historique_commande.append(com)

        return retcode

    def calculer_flux_solaires(self, 
                               geo = None,
                               nb_transmission = -1, 
                               fichier_meteo = None):
        """
        macro : calcul des flux solaires direct, diffus, et total
        """
        
        chrono = Chrono('calculer_flux_solaires', 'solCommande') 
        self.calculer_flux_sol_direct(nb_transmission = nb_transmission, 
                                      fichier_meteo = fichier_meteo)
        self.calculer_flux_sol_diffus(nb_transmission = nb_transmission, 
                                      fichier_meteo = fichier_meteo)
        self.calculer_flux_sol_total(geo = geo)
        
        chrono.fin()

    def calculer_fac_form(self, 
                        transparence = False, 
                        epsilon = 0.001):

        """
        calcul des facteurs de forme
        
        appelle la fonction externe 'facform' si transparence = False (defaut)
        'facform_ciel' si transparence = True
        """
        
        chrono = Chrono('calculer_fac_form', 'solCommande')
        if transparence:
            com = ['facform_lum',
                 self.scene_cir,
                 self.masque_cir,
                 self.carac['transmittance'],
                 str(0),
                 self.facform,
                 self.carac['ffIn'],
                 self.carac['ffOut'],
                 str(epsilon)]


        else:
            com = ['facform',
                 self.scene_cir,
                 self.masque_cir,
                 str(0),
                 self.facform,
                 self.carac['ffIn'],
                 self.carac['ffOut'],
                 str(epsilon)]

        retcode = subprocess.call(com,
                                stdout = self.fichier_sortie, 
                                stderr = self.fichier_sortie)
        self.historique_commande.append(com)
                
        chrono.fin()

        return retcode
    
    def calculer_fac_form_ciel(self):
        """
        calcul du facteur de vue du ciel
        """
        com = ['facform_ciel',
             self.scene_cir,
             self.masque_cir,
             self.ciel_cir,
               str(self.param['angle_vision']),
             self.carac['ffSky']]

        retcode = subprocess.call(com)
        self.historique_commande.append(com)

        return retcode
    
    def calculer_radiosite(self, 
                           geo = None,
                           type_arret = 'p',
                           valeur_arret = '5'):
        """
        calculer les rayonnements après interreflexions par la méthode
        des radiosités
        """
        
        chrono = Chrono('calculer_radiosite', 'solCommande')

        if not os.path.isfile(self.carac['surface']+'.val'):
            self.calculer_surface()

        for suffix in self.liste_ts_sol:
            print(time.ctime(), ' >> radiosite: ', suffix, end=' ')
            chro_chro = Chrono('chro', 'chro', verbose = False)
            com = ['radiosite',
                    self.var['flux_sol_total'] + '_' + suffix,
                    self.carac['albedo'],
                    self.facform,
                    self.carac['surface'],
                    self.var['ecl_inc_total'] + '_' + suffix,
                    self.var['ecl_ref_total'] + '_' + suffix,
                    self.var['ecl_abs_total'] + '_' + suffix,
                    type_arret,
                    valeur_arret]
            retcode1 = subprocess.call(com,
                                       stdout = self.poubelle, 
                                       stderr = self.fichier_sortie)
            self.historique_commande.append(com)
            
            if geo:
                ecl_inc_total = read_val(self.var['ecl_inc_total'] + '_' + suffix, geom=geo)
                flux_sol_direct = read_val(self.var['flux_sol_direct'] + '_' + suffix, geom=geo)
                ecl_diffus = ecl_inc_total- flux_sol_direct
                write_val(self.var['ecl_diffus'] + '_' + suffix, geo, ecl_diffus)
                retcode2 = 0
            else:
                com = ['val_op_val',
                       self.var['ecl_inc_total'] + '_' + suffix,
                       '-',
                       self.var['flux_sol_direct'] + '_' + suffix,
                       self.var['ecl_diffus'] + '_' + suffix]
                
                retcode2 = subprocess.call(com,
                                           stdout = self.poubelle,
                                           stderr = self.fichier_sortie)
                self.historique_commande.append(com)        
                
                chro_chro.fin()
                print(chro_chro.duree)

        chrono.fin()
                                           
        return retcode1, retcode2

    def simulation_Ts_Energie_Bat(self, 
                                  suffixe_avant, 
                                  suffixe_apres, 
                                  meteo = False, 
                                  nom_commande = COM_ENER_BAT,
                                  terminal = True):
        
        if meteo:
            self.definir_meteo_ts(suffixe_apres, t_ciel = False)
        
        com = [nom_commande,
               str(self.pas_de_temps),
               self.carac['surface'], # 2
               self.facform,          # 3
               self.carac['ffSky'],   # 4
               self.var['Tair'],      # 5
               self.var['HR'],        # 6
               self.var['hc'],        # 7
               str(self.flux_ir),# 8
               self.var['flux_sol_direct'] + '_' + suffixe_apres, # 9
               self.var['ecl_diffus'] + '_' + suffixe_apres,  # 10
               self.paroi,  # 11
               self.carac['no_classe'],  # 12
               self.carac['no_paroi'], # 13
               self.carac['albedo'], # 14
               self.carac['emissivite'], # 15
               self.carac['transmittance_bat'], # 16
               self.carac['evaporation'], # 17
               self.meteo_sol, # 18
               self.carac['niveau'], # 19
               self.carac['entree_air'], # 20
               str(self.param['temp_consigne']), # 21
               str(self.param['taux_occupation']), # 22
               str(self.param['puissance_sensible']), # 23
               str(self.param['regime_thermique']), # 24
               str(self.param['saison']), # 25
               self.var['Tse'] + '_' + suffixe_avant, # 26
               self.var['Tn1'] + '_' + suffixe_avant, # 27
               self.var['Tn2'] + '_' + suffixe_avant, # 28
               self.var['Tnoeud'] + '_' + suffixe_avant, # 29
               self.var['Tse'] + '_' + suffixe_apres, # 30
               self.var['Tn1'] + '_' + suffixe_apres, # 31
               self.var['Tn2'] + '_' + suffixe_apres, # 32
               self.var['Tnoeud'] + '_' + suffixe_apres, # 33
               self.option_resul] # 34

        verf = [self.carac['surface']+'.val', # 2e
               self.facform+'.fac',          # 3
               self.carac['ffSky']+'.val',   # 4
               self.var['Tair']+'.val',      # 5
               self.var['HR']+'.val',        # 6
               self.var['hc']+'.val',        # 7
               self.var['flux_sol_direct'] + '_' + suffixe_apres+'.val', # 9
               self.var['ecl_diffus'] + '_' + suffixe_apres+'.val',  # 10
               self.paroi+'.txt',  # 11
               self.carac['no_classe']+'.val',  # 12
               self.carac['no_paroi']+'.val', # 13
               self.carac['albedo']+'.val', # 14
               self.carac['emissivite']+'.val', # 15
               self.carac['transmittance_bat']+'.val', # 16
               self.carac['evaporation']+'.val', # 17
               self.meteo_sol+'.txt', # 18
               self.carac['niveau']+'.val', # 19
               self.carac['entree_air']+'.val', # 20
               self.var['Tse'] + '_' + suffixe_avant+'.val', # 21
               self.var['Tn1'] + '_' + suffixe_avant+'.val', # 22
               self.var['Tn2'] + '_' + suffixe_avant+'.val', # 23
               self.var['Tnoeud'] + '_' + suffixe_avant+'.txt', # 24
               self.option_resul +'.txt']
               
        for ligne in verf:
            if not os.path.isfile(ligne):
                stop = True
                print(ligne, 'n existe pas')
                
        self.bak.append([time.time(), com])
        
        texte = '\n\n SIMULATION_TS_ENERGIEBAT \n\n calcul pas de temps %s à %s\n\n' % (suffixe_avant, suffixe_apres)
        for mot in com:
            texte += str(mot) + ' '
                
        texte += '\n\n' 
        
        self.fichier_sortie.write(texte)

        self.historique_commande.append(com)
        if terminal:
            retcode = subprocess.call(com,
                                      stdout = self.fichier_sortie, 
                                      stderr = self.fichier_sortie)
        else:
            retcode = subprocess.call(com)
                                          
        return retcode
    
    def _creer_dic_commande_Ts_EnergieBat_common(
        self,
        suffixe_avant,
        suffixe_apres,
        simulation_batiment,
        simulation_vegetation,
        *,
        include_ecl_inc_tot: bool,
        include_tp_outputs: bool,
        include_water_outputs: bool,
        include_extra_nodes: bool,
    ):
        dic_com = {}

        dic_com["simulation_batiment"] = str(int(simulation_batiment))
        dic_com["simulation_vegetation"] = str(int(simulation_vegetation))

        dic_com["dt"] = str(self.pas_de_temps)
        dic_com["flux_atm"] = str(self.flux_ir)
        dic_com["ps_equip"] = str(self.param["puissance_sensible"])
        dic_com["alpha_arg"] = str(self.param["regime_thermique"])
        dic_com["saison"] = str(self.param["saison"])
        dic_com["taux_occup"] = str(self.param["taux_occupation"])
        dic_com["T10consigne"] = str(self.param["temp_consigne"])

        dic_com["paroi"] = f"{self.paroi}.txt"
        dic_com["sortie_aux"] = f"{self.option_resul}.txt"
        dic_com["sol"] = f"{self.meteo_sol}.txt"

        dic_com["fform"] = f"{self.facform}.fac"

        dic_com["classe_paroi"] = f"{self.carac['no_classe']}.val"
        dic_com["id_paroi"] = f"{self.carac['no_paroi']}.val"

        dic_com["surface"] = f"{self.carac['surface']}.val"
        dic_com["fciel"] = f"{self.carac['ffSky']}.val"
        dic_com["albedo"] = f"{self.carac['albedo']}.val"
        dic_com["transmission"] = f"{self.carac['transmittance_bat']}.val"
        dic_com["emissivite"] = f"{self.carac['emissivite']}.val"

        dic_com["f_evap_sol"] = f"{self.carac['evaporation']}.val"
        dic_com["LAI"] = f"{self.carac['LAI']}.val"

        if include_water_outputs:
            dic_com["f_h_eau_sol"] = f"{self.carac['h_eau_sol']}.val"
            dic_com["f_dt_eau_sol"] = f"{self.carac['dt_arrosage_sol']}.val"

        dic_com["Cf"] = f"{self.carac['Cf']}.val"
        dic_com["Ca"] = f"{self.carac['Ca']}.val"
        dic_com["h_ainf"] = f"{self.carac['h_ainf']}.val"
        dic_com["h_pa"] = f"{self.carac['h_pa']}.val"

        dic_com["in_air"] = f"{self.carac['entree_air']}.val"
        dic_com["niveau"] = f"{self.carac['niveau']}.val"

        dic_com["flux_sol_dir"] = f"{self.var['flux_sol_direct']}_{suffixe_apres}.val"
        dic_com["flux_sol_diff"] = f"{self.var['flux_sol_diffus']}_{suffixe_apres}.val"

        if include_ecl_inc_tot:
            dic_com["ecl_inc_tot"] = f"{self.var['ecl_inc_total']}_{suffixe_apres}.val"

        dic_com["hc_ext"] = f"{self.var['hc']}.val"
        dic_com["Text"] = f"{self.var['Tair']}.val"
        dic_com["w_ext"] = f"{self.var['HR']}.val"
        dic_com["vit"] = f"{self.carac['v']}.val"

        dic_com["TSext_init"] = f"{self.var['Tse']}_{suffixe_avant}.val"

        if include_tp_outputs:
            dic_com["TPint1_init"] = f"{self.var['Tp1']}_{suffixe_avant}.val"
            dic_com["TPint2_init"] = f"{self.var['Tp2']}_{suffixe_avant}.val"

        dic_com["TNint1_init"] = f"{self.var['Tn1']}_{suffixe_avant}.val"
        dic_com["TNint2_init"] = f"{self.var['Tn2']}_{suffixe_avant}.val"

        dic_com["TSext"] = f"{self.var['Tse']}_{suffixe_apres}.val"

        if include_tp_outputs:
            dic_com["TPint1"] = f"{self.var['Tp1']}_{suffixe_apres}.val"
            dic_com["TPint2"] = f"{self.var['Tp2']}_{suffixe_apres}.val"

        dic_com["TNint1"] = f"{self.var['Tn1']}_{suffixe_apres}.val"
        dic_com["TNint2"] = f"{self.var['Tn2']}_{suffixe_apres}.val"

        if include_extra_nodes:
            for key in range(3, 22):
                dic_com[f"TN{key}_init"] = f"{self.var[f'TN{key}']}_{suffixe_avant}.val"
                dic_com[f"TN{key}"] = f"{self.var[f'TN{key}']}_{suffixe_apres}.val"

        if include_water_outputs:
            dic_com["h_eau_sol_tot_init"] = f"{self.var['h_eau_sol_tot']}_{suffixe_avant}.val"
            dic_com["h_eau_sol_tot"] = f"{self.var['h_eau_sol_tot']}_{suffixe_apres}.val"

        dic_com["Ta_init"] = f"{self.var['Ta']}_{suffixe_avant}.val"
        dic_com["Tse_veg_init"] = f"{self.var['Tse_veg']}_{suffixe_avant}.val"
        dic_com["Ta"] = f"{self.var['Ta']}_{suffixe_apres}.val"
        dic_com["Tse_veg"] = f"{self.var['Tse_veg']}_{suffixe_apres}.val"

        dic_com["Tnoeuds_init"] = f"{self.var['Tnoeud']}_{suffixe_avant}.txt"
        dic_com["Tnoeuds_out"] = f"{self.var['Tnoeud']}_{suffixe_apres}.txt"

        return dic_com
    
    def ecrire_commande_Ts_EnergieBat(
        self,
        suffixe_avant,
        suffixe_apres,
        simulation_batiment=True,
        simulation_vegetation=True,
    ):
        options = self._get_ts_energie_bat_options()

        dic_com = self._creer_dic_commande_Ts_EnergieBat_common(
            suffixe_avant=suffixe_avant,
            suffixe_apres=suffixe_apres,
            simulation_batiment=simulation_batiment,
            simulation_vegetation=simulation_vegetation,
            **options,
        )

        lst_cle = self._build_ts_energie_bat_key_order(**options)

        self._ecrire_commande_Ts_EnergieBat_common(dic_com, lst_cle)

    def _get_ts_energie_bat_options(self):
        return {
            "include_ecl_inc_tot": self.profile.enable_ecl_inc_tot,
            "include_tp_outputs": self.profile.enable_tp_outputs,
            "include_water_outputs": self.profile.enable_water_descriptors,
            "include_extra_nodes": self.profile.enable_extra_transient_nodes,
        }
    
    def _build_ts_energie_bat_key_order(
        self,
        *,
        simulation_batiment: bool,
        simulation_vegetation: bool,
        include_ecl_inc_tot: bool = True,
        include_tp_outputs: bool,
        include_water_outputs: bool,
        include_extra_nodes: bool,
    ):
        lst_cle = [
            "simulation_batiment",
            "simulation_vegetation",
            "dt",
            "flux_atm",

            "paroi",
            "sortie_aux",
            "sol",

            "fform",

            "classe_paroi",
            "id_paroi",

            "surface",
            "fciel",
            "albedo",
            "transmission",
            "emissivite",

            "f_evap_sol",
            "LAI",
        ]

        if include_water_outputs:
            lst_cle.extend([
                "f_h_eau_sol",
                "f_dt_eau_sol",
            ])

        lst_cle.extend([
            "flux_sol_dir",
            "flux_sol_diff",
        ])

        if include_ecl_inc_tot:
            lst_cle.append("ecl_inc_tot")

        lst_cle.extend([
            "hc_ext",
            "Text",
            "w_ext",

            "TSext_init",
        ])

        if include_tp_outputs:
            lst_cle.extend([
                "TPint1_init",
                "TPint2_init",
            ])

        lst_cle.extend([
            "TNint1_init",
            "TNint2_init",
            "TSext",
        ])

        if include_tp_outputs:
            lst_cle.extend([
                "TPint1",
                "TPint2",
            ])

        lst_cle.extend([
            "TNint1",
            "TNint2",
        ])

        if include_extra_nodes:
            for key in range(3, 22):
                lst_cle.append(f"TN{key}_init")
                lst_cle.append(f"TN{key}")

        if include_water_outputs:
            lst_cle.extend([
                "h_eau_sol_tot_init",
                "h_eau_sol_tot",
            ])

        if simulation_batiment:
            lst_cle.extend([
                "ps_equip",
                "alpha_arg",
                "saison",
                "taux_occup",
                "T10consigne",

                "in_air",
                "niveau",

                "Tnoeuds_init",
                "Tnoeuds_out",
            ])

        if simulation_vegetation:
            lst_cle.extend([
                "vit",
                "Cf",
                "Ca",
                "h_ainf",
                "h_pa",
                "Ta_init",
                "Tse_veg_init",
                "Ta",
                "Tse_veg",
            ])

        return lst_cle
    
    def _ecrire_commande_Ts_EnergieBat_common(self, dic_com, lst_cle):
        texte = ""
        for cle in lst_cle:
            texte += f"{dic_com[cle]}\n"

        ecrire_fichier(self.fichier_commande, texte)

    def simulation_Ts_EnergieBat_new(self,
                                     suffixe_avant, 
                                     suffixe_apres,
                                     simulation_batiment = True,
                                     simulation_vegetation = True,
                                     meteo = False, 
                                     nom_commande = None,
                                     verbose = False,
                                     args_plus = None,
                                     terminal = False):
        

        chrono = Chrono('simulation_Ts_EnergieBat', 'solCommande', verbose= verbose)
        if meteo:
            self.definir_meteo_ts(suffixe_apres, t_ciel = False)

        self.ecrire_commande_Ts_EnergieBat(suffixe_avant, suffixe_apres, simulation_batiment, simulation_vegetation)

        com = [nom_commande,
               self.fichier_commande]
        self.bak.append([time.time(), com])
        
        texte = '\n\n SIMULATION_TS_ENERGIEBAT \n\n calcul pas de temps %s à %s\n\n' % (suffixe_avant, suffixe_apres)
        for mot in com:
            texte += str(mot) + ' '
                
        texte += '\n\n' 
        
        self.fichier_sortie.write(texte)

        if terminal:
            retcode = subprocess.call(com)
        else:
            retcode = subprocess.call(com,
                                      stdout = self.fichier_sortie, 
                                      stderr = self.fichier_sortie)
        self.historique_commande.append(com)
        if (simulation_batiment):
            self.check_nan(self.var['Tnoeud'] + '_' + suffixe_apres + '.txt')
        
        chrono.fin()    
        return retcode

    def _creer_nom_descripteur(self) -> None:
        for descr in self.liste_carac:
            self.carac[descr] = JOIN(self.chemin_entree, descr)

    def _creer_nom_variable(self) -> None:
        for var in self.liste_variables:
            if var in self.variables_clo:
                if not os.path.isdir(JOIN(self.chemin_clo, var)):
                    os.makedirs(JOIN(self.chemin_clo, var))
                self.var[var] = JOIN(JOIN(self.chemin_clo, var), var)

            elif var in self.variables_transient:
                if not os.path.isdir(JOIN(self.chemin_transitoire, var)):
                    os.makedirs(JOIN(self.chemin_transitoire, var))
                self.var[var] = JOIN(JOIN(self.chemin_transitoire, var), var)

            elif var in self.variables_glo:
                if not os.path.isdir(JOIN(self.chemin_transitoire, var)):
                    os.makedirs(JOIN(self.chemin_transitoire, var))
                self.var[var] = JOIN(JOIN(self.chemin_transitoire, var), var)

        self.var_bak = self.var.copy()

    def _creer_nom_variable_confort(self) -> None:
        for var in self.liste_variables_confort:
            if not os.path.isdir(JOIN(self.chemin_confort, var)):
                os.makedirs(JOIN(self.chemin_confort, var))
            self.var_confort[var] = JOIN(self.chemin_confort, var, var)

    def _composer_nom(self, nom_val, jour, heure = None):
        """
        renvoi le nom de fichier du descripteur -nom_val- avec la date et 
        l'heure : nomval_jj_mm_hhHmm'
        """
        nom_sortie = nom_val+'_' + str(self.liste_jours[jour][0])+'_'+\
            str(self.liste_jours[jour][1]) 

        if heure : 
            nom_sortie += '_'+heure 

        return nom_sortie
    
    def _composerJJMM(self, jour):
        """
        renvoie une chaine représentant l'écriture de la date dans la 
        convention des commande solene : 'jj/mm'
        """
        jour_heure = str(self.liste_jours[jour][0])+'/'+\
            str(self.liste_jours[jour][1])

        return jour_heure
    
    def _definir_meteo_ts(self, suffixe, t_ciel = False):
        """
        definie la meteo correspondant au suffixe (Tmeteo et flux_ir)
        calcule flux_ir si la donnee meteo est t_ciel
        """
        self.t_meteo = self.dic_meteo[suffixe]['T']
        if t_ciel:
            T_ciel = self.dic_meteo[suffixe]['Tciel']
            self.flux_ir = self.sigma *(273.15 + T_ciel)**4
        else:
            self.flux_ir = self.dic_meteo[suffixe]['flux_ir']

    def _check_nan(self, nom_fichier):
        """
        verifie que le fichier 'nom_fichier' ne contient pas de 'nan'
        """
        if 'nan' in open(nom_fichier).read():
            return 0
        else:
            return 1