#!/usr/bin/python
# -*- coding: utf-8 -*-

"""@package satCommand
======================================================================
= =                       solCommande.py                           = =
======================================================================

Définition d'une classe SolCommand de contrôle de simulation de Solene

Version 0.02

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr
    
création : 14/01/2010
maj :      06/05/2010

clean :    28/07/2010
"""
import subprocess
import os
import time

from infrastructure.geometry.legacy.utils import Chrono, ecrire_fichier
from infrastructure.geometry.legacy.solFile import *

JOIN = os.path.join
COM_ENER_BAT = 'simulation_Ts_EnergieBat_laurent'
COM_ENERGIE_BAT_VEG = 'simulation_Ts_EnergieBat_VEG'
COM_ENERGIE_BAT_NEW = '/home/laurent/solenetb/src/trunk/solene.thermo-radiatif.core/exe/simulation_Ts_EnergieBat.exe'


LISTE_OPTION = """Tse	1
Tn1	0
Tn2	0
Tsi1	0
Tsi2	0
GLO_Ciel_Emis	0
GLO_Ciel_Recu	0
GLO_Ciel_Net 	1
GLO_Scene_Emis	0
GLO_Scene_Recu	0
GLO_Scene_Net	0
GLO_Total_Emis	0
GLO_Total_Recu	0
GLO_Total_Net	1
Conv		1
Sol_Ciel_Reflechi 	1
Sol_Net			0
Sol_Scene_Apport	0
        """

NOM_FICHIER_METEO_DIFFUS = 'meteo_diffus'
NOM_FICHIER_METEO_DIRECT = 'meteo_direct'

# caracteristiques : entrees des simulations
CARAC_FACE = ['albedo', 
              'transmittance',
              'transmittance_bat',
              'emissivite',
              'h_int',
              'Tfix',
              'Tint',
              'no_paroi',
              'no_paroi_tr',
              'no_classe']
        
CARAC_TRIANGLE = ['surface',
                  'ffIn',
                  'ffOut',
                  'ffSky',
                  'evaporation',
                  'niveau',
                  'entree_air',
                  'Ca',
                  'Cf',
                  'h_pa',
                  'h_ainf',
                  'v',
                  'LAI',
                  'h_eau_sol',
                  'dt_arrosage_sol']

# variables : sorties des simulations ou entrees resultats de simulations
#             (ex hc)

VARIABLES_CLO = ['flux_sol_direct',
                 'flux_sol_diffus',
                 'flux_sol_total',
                 'ecl_inc_direct',
                 'ecl_inc_diffus',
                 'ecl_inc_total',
                 'ecl_ref_direct',
                 'ecl_ref_diffus',
                 'ecl_ref_total',
                 'ecl_abs_direct',
                 'ecl_abs_diffus',
                 'ecl_abs_total',
                 'ecl_diffus']
                 
VARIABLES_GLO = ['GLO_Ciel_Net',
                 'GLO_Scene_Net',
                 'GLO_Total_Net',
                 'flux_latent',
                 'flux_eau',
                 'flux_rad_arbre',
                 'humidite_relative',
                 'flux_convectif']
                 
VARIABLES_TRANSITOIRE = ['Tse',
                         'Tp1',
                         'Tp2',  
                         'Tn1',
                         'Tn2',
                        'Tn3',
                         'Ta',
                         'Tse_veg',
                         'Tnoeud',
                         'Tinit',
                         'hc',
                         'Tair',
                         'HR',
                         'TN3',
                         'TN4',
                         'TN5',
                         'TN6',
                         'TN7',
                         'TN8',
                         'TN9',
                         'TN10',
                         'TN11',
                         'TN12',
                         'TN13',
                         'TN14',
                         'TN15',
                         'TN16',
                         'TN17',
                         'TN18',
                         'TN19',
                         'TN20',
                         'TN21',
                         'h_eau_sol_tot']

VARIABLES_CONFORT = ['flux_sol_direct',
                     'flux_sol_diffus',
                     'albedo',
                     'vitesse_air',
                     'temperature_air',
                     'MRT']

class SolCommand():
    """
    Classe de simulation solene
    
    les chemins vers les fichiers d'entree et de sortie sont definis par défaut
    """
    def __init__(self,
                 chemin_sim,
                 nom_cas,
                 n_ciel = 3):
        
        """
        Constructeur de simulation solene
        """
        
        self.nom_cas = nom_cas
        self.chemin_simul_sol = chemin_sim
        
        self.chemin_ciel = JOIN(self.chemin_simul_sol, 'ciel')
        self.chemin_entree = JOIN(self.chemin_simul_sol, 'entree')
        self.chemin_sortie = JOIN(self.chemin_simul_sol, 'sortie')
        
        
        self.chemin_clo = JOIN(self.chemin_sortie, 'CLO')
        self.chemin_transitoire = JOIN(self.chemin_sortie, 'transitoire')
        self.chemin_ener = JOIN(self.chemin_sortie, 'ener')
        self.chemin_confort = JOIN(self.chemin_sortie, 'confort')
        
        self.scene_cir = JOIN(self.chemin_simul_sol, 'scene_triangle')
        self.masque_cir = JOIN(self.chemin_simul_sol, 'scene_masque')
 
        self.sol_confort_cir = JOIN(self.chemin_confort, 'sol_confort')
        self.bonhomme_confort_cir = JOIN(self.chemin_confort, 'bonhomme_confort')
        self.bonhomme_confort_surface = JOIN(self.chemin_confort, 'bonhomme_confort_surface')

        self.chemin_masque = JOIN(self.chemin_clo, 'masque')
        self.masque_mas = JOIN(self.chemin_masque, 'masque')
        
        self.param = {}
        self.param['latitude'] = 46
        self.param['longitude'] = -1
        self.param['meridien'] = 0

        ## DEPRECATED
        self.latitude = 46
        self.longitude = -1
        self.meridien = 0
        self.liste_jours = [[10, 7, '10:00', '16:00', '01:00']]
        

        self.ciel_cir = JOIN(self.chemin_ciel, 'ciel')
        self.ciel_angle_solide_val = JOIN(self.chemin_ciel, 'angleSolideCiel')
        self.luminance_ciel_val = JOIN(self.chemin_ciel, 'luminance')

        self.chemin_meteo_diffus = JOIN(self.chemin_ciel, 
                                        NOM_FICHIER_METEO_DIFFUS)
        self.chemin_meteo_direct = JOIN(self.chemin_ciel, 
                                        NOM_FICHIER_METEO_DIRECT)

        self.historique_commande = []

        self.fichier_commande = JOIN(self.chemin_entree, 'commande_sim_ts')
        
        # liste des suffixes
        self.liste_ts_sol = []

        # nom des descripteurs caracteristique materiaux
        self.liste_carac = CARAC_FACE + CARAC_TRIANGLE
        self.carac = {}
        self._creer_nom_descripteur()
        
        # nom des descripteurs resultats de simulations
        self.liste_variables = (VARIABLES_CLO + 
                                VARIABLES_GLO + 
                                VARIABLES_TRANSITOIRE)
        self.liste_variables_confort = VARIABLES_CONFORT
        self.var = {}
        self.var_confort = {}
        self._creer_nom_variable()
        self._creer_nom_variable_confort()

        # nom de la matrice de facteur de forme
        self.facform = JOIN(self.chemin_simul_sol, 'facform')

        # nom des valeurs thermiques
        self.t_meteo = 25.25
        self.pas_de_temps = 3600
        self.opt_interreflexions = 1
        self.flux_ir = 350.73
        self.option_calc = JOIN(self.chemin_entree, 'option_calc')
        self.option_resul = JOIN(self.chemin_entree, 'option_resul')
        self.meteo_sol = JOIN(self.chemin_entree, 'meteo_sol')
        self.paroi = JOIN(self.chemin_entree, 'paroi')
        self.paroi_transitoire = JOIN(self.chemin_entree, 'paroi_tr')
        self.sigma = 5.67e-8
        self.coeff_veg = JOIN(self.chemin_entree, 'coeff_veg')
        
        # nom des valeurs energetiques
        self.param['temp_consigne'] = 20
        self.param['taux_occupation'] = 0.063
        self.param['puissance_sensible'] =  0
        self.param['regime_thermique'] = 1 #  0: force, 1: libre, 2: mixte
        self.param['saison'] = 0                  #  0: ete,   1: hiver 

        ## DEPRECATED
        self.temp_consigne = 20
        self.taux_occupation = 0.063
        self.puissance_sensible =  0
        self.regime_thermique = 1 #  0: force, 1: libre, 2: mixte
        self.saison = 0                  #  0: ete,   1: hiver 
        
        # parametre par defaut:

        self.param['angle_vision'] = 89.9
        self.param['transparence'] = True
        self.param['opt_lumiere'] = 0 
        self.param['delta_perez'] = 0.12
        self.param['epsilon_perez'] = 6.3

        ## DEPRECATED
        self.angle_vision = 89.9
        self.transparence = True
        self.opt_lumiere = 0 
        self.delta_perez = 0.12
        self.epsilon_perez = 6.3

        # fichier sortie
        suffix = '%d%.2d%.2d-%.2d%.2d'%time.localtime()[:5]

        self.chemin_fichier_sortie = JOIN(self.chemin_simul_sol, 
                                          'stdout-' + suffix)
        self.fichier_sortie = open(self.chemin_fichier_sortie,'w')
        
        # fichier poubelle radiosite
        self.chemin_poubelle = JOIN('/', 'tmp', 'poubelle_radiosite')
        self.poubelle = open(self.chemin_poubelle, 'w')


        self.bak = []
        self.creer_cas()
        

    def creer_cas(self):
        """
        création de l'arborescence si elle n'existe pas
        """
        print('\n  CREATION DU CAS SOLENE')
        
        if not os.path.isdir(self.chemin_simul_sol):
            os.mkdir(self.chemin_simul_sol)
            print('\t creation repertoire -simulSol-')
            print('\t -> ', self.chemin_simul_sol)
        else: 
            print('\t -simulSol- existe deja')

        if not os.path.isdir(self.chemin_entree):
            os.mkdir(self.chemin_entree)
            print('\t creation repertoire -entree-')
            
        if not os.path.isdir(self.chemin_sortie):
            os.mkdir(self.chemin_sortie)
            print('\t creation repertoire -sortie-')

        if not os.path.isdir(self.chemin_clo):
            os.mkdir(self.chemin_clo)
            print('\t creation repertoire -CLO-')
            
        if not os.path.isdir(self.chemin_transitoire):
            os.mkdir(self.chemin_transitoire)
            print('\t creation repertoire -transitoire-')
                
        if not os.path.isdir(self.chemin_ener):
            os.mkdir(self.chemin_ener)
            print('\t creation repertoire -ener-')
                
        if not os.path.isdir(self.chemin_ciel):
            os.mkdir(self.chemin_ciel)
            print('\t creation repertoire -Ciel-')
            self.creer_ciel()

        if not os.path.isdir(self.chemin_masque):
            os.mkdir(self.chemin_masque)
            print('\t creation repertoire -CLO/masque-')
            self.creer_ciel()

    def creer_cas_confort(self):
        if not os.path.isdir(self.chemin_confort):
            os.mkdir(self.chemin_confort)
            print('\t creation repertoire -Ciel-')
        

    def _creer_nom_descripteur(self):
        """
        cree le chemin des descripteurs definissant les caracteristiques
        """
        for descr in self.liste_carac:
            self.carac[descr] = JOIN(self.chemin_entree, descr)
        
    def _creer_nom_variable(self):
        """
        definis le chemin où sont stockées les variables suivant qu'elles soit 
        lié aux courtes longueurs d'onde, à la simulation transitoire 
        ou energetique
        """
        for var in self.liste_variables:
            if var in VARIABLES_CLO:
                if not os.path.isdir(JOIN(self.chemin_clo, var)):
                    os.makedirs(JOIN(self.chemin_clo, var))
                self.var[var] = JOIN(JOIN(self.chemin_clo, var), var)

            elif var in VARIABLES_TRANSITOIRE:
                if not os.path.isdir(JOIN(self.chemin_transitoire, var)):
                    os.makedirs(JOIN(self.chemin_transitoire, var))
                self.var[var] = JOIN(JOIN(self.chemin_transitoire, var), var)

            elif var in VARIABLES_GLO:
                if not os.path.isdir(JOIN(self.chemin_transitoire, var)):
                    os.makedirs(JOIN(self.chemin_transitoire, var))
                self.var[var] = JOIN(JOIN(self.chemin_transitoire, var), var)

        self.var_bak = self.var.copy()
            #elif var in variables_ener:
            #    self.variables[var] = JOIN(self.chemin_ener, var)

    def _creer_nom_variable_confort(self):
        for var in self.liste_variables_confort:
            if not os.path.isdir(JOIN(self.chemin_confort, var)):
                os.makedirs(JOIN(self.chemin_confort, var))
            self.var_confort[var] = JOIN(self.chemin_confort, var, var)
                                     

#=====================================================================
# fonction "auxiliaires"

    def recuperer_param_from_dic(self, dic_param):
        """
        definir les param solene a partir des donnees extraite d'un fichier
        parametre xml parse par XmlFile.py
        
        """
        for cle, it in list(dic_param.items()):
            if cle in self.param:
                self.param[cle] = it['value']

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

    def _creer_liste_heures(self, jour):
        """
        crée la liste d'heure pour les suffixes des descripteurs
        lus et ecrits par solene [HH:MM]
        suivant une description au format d'entree solene :
            (debut, fin, pas)
        """
        debut = self.liste_jours[jour][2]
        fin = self.liste_jours[jour][3]
        pas = self.liste_jours[jour][4]

        debut_min = int(debut[0:2])*60 + int(debut[3:5])
        fin_min = int(fin[0:2])*60 + int(fin[3:5])
        pas_min = int(pas[0:2])*60 + int(pas[3:5])

        liste_heures = []
        for heure in range(debut_min, fin_min + pas_min, pas_min):
            minutes = str(heure%60)
            heures = str(int(heure/60))
            if len(minutes) == 1 : 
                minutes += '0'
                
            if len(heures) == 1:
                heures = '0' + heures
                
            liste_heures.append(heures+'H'+minutes)

        return liste_heures

    def _creer_liste_heures_tot(self):
        """
        Creates a list containing all suffixes for each descriptor
        """
        liste_suffix = []
        for i_jour in self.liste_jours:
            jour = i_jour[0]
            mois = i_jour[1]
            suffix = '_%s_%s_' % (jour, mois)
            liste_heure = self._creer_liste_heures(i_jour)

            for heure in liste_heure:
                liste_suffix.append(suffix+heure)

        self.liste_ts_sol = liste_suffix
        
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
        
    def definir_meteo_ts(self, suffixe, t_ciel = False):
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
            
    def tsv2tl(self, jour, mois, heure, heure_ete):
        """
        renvoi le temps solaire vrai en fonction du temps local
        utilise la fonction solene tsv_tl
        """
        com = ['tsv_tl',
               str(self.param['latitude']),
               str(self.param['longitude']),
               str(self.param['meridien']),
               str(jour),
               str(mois),
               str(heure), 
               str(heure_ete)]
               
        pop = subprocess.Popen(com, stdout = subprocess.PIPE)
        output = pop.stdout.readlines()
        
        for ligne in output:
            if 'Heure legale' in ligne:
                ttt = ligne.split()
                return [int(ttt[3]), int(ttt[5])]
                
    def tl2tsv(self, jour, mois, heure, heure_ete):
        """
        renvoi le temps local en fonction du temps solaire vrai
        utilise la fonction solene tl_tsv
        """

        com = ['tl_tsv',
               str(self.param['latitude']),
               str(self.param['longitude']),
               str(self.param['meridien']),
               str(jour),
               str(mois),
               str(heure), 
               str(heure_ete)]
               
        pop = subprocess.Popen(com, stdout = subprocess.PIPE)
        output = pop.stdout.readlines()
        
        for ligne in output:
            if 'Heure solaire' in ligne:
                ttt = ligne.split()
                return [int(ttt[3]), int(ttt[5])]

    def info_solaire_autre(self, jour, mois, heure, minute):
        """
        renvoi l'azimut et la hauteur solaire en degre
        (utilise l'executable perso 'print_info_soleil')
        """
        com = ['print_info_soleil',
               str(self.param['latitude']),
               str(jour),
               str(mois),
               str(heure), 
               str(minute)]
               
        pop = subprocess.Popen(com, stdout = subprocess.PIPE)
        output = pop.stdout.readlines()
        
        for ligne in output:
            if 'azimut' in ligne:
                ttt = ligne.split()
                azimut = float(ttt[-1])
            if 'hauteur' in ligne:
                ttt = ligne.split()
                hauteur = float(ttt[-1])
                
        return azimut, hauteur

    def info_solaire(self, i_ts):
        """
        renvoi l'azimut et la hauteur solaire en degre
        (utilise l'executable perso 'print_info_soleil')
        """
        jour, mois, hhmm = self.liste_ts_sol[i_ts].split('_')
        heure, minute = hhmm.split('H')
        
        com = ['print_info_soleil',
               str(self.param['latitude']),
               str(jour),
               str(mois),
               str(heure), 
               str(minute)]
               
        pop = subprocess.Popen(com, stdout = subprocess.PIPE)
        output = pop.stdout.readlines()
        
        for ligne in output:
            if 'azimut' in ligne:
                ttt = ligne.split()
                azimut = round(float(ttt[-1]), 3)
            if 'hauteur' in ligne:
                ttt = ligne.split()
                hauteur = round(float(ttt[-1]), 3)
                
        return azimut, hauteur


    def check_nan(self, nom_fichier):
        """
        verifie que le fichier 'nom_fichier' ne contient pas de 'nan'
        """
        if 'nan' in open(nom_fichier).read():
            return 0
        else:
            return 1
        
#=====================================================================
# appel de fonctions solene
#

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

    def pivoter_scene(self, angle, fichier_in, fichier_out = None):
        """
	rotation de -fichier_in- suivant l'axe Z
        """
        if not fichier_out:
            fichier_out = fichier_in

        com = ['rot', 'Z', str(angle), fichier_in, fichier_out]
        subprocess.call(com)

        
    def calculer_fichier_masque(self):
        """
        calcul du fichier masque utilisant la fonction solene 
        masques_sol

        util si le calcul du flux incident se fait avec des flux CLO
        meteo en entree
        """
        for i, jour in enumerate(self.liste_jours):
            print('= calcul jour : %s' % jour)
            
            pas_min = str(int(jour[4][:2])*60 
                          +int(jour[4][3:]))
            
            com = ['masques_sol',
                   self.scene_cir,
                   self.masque_cir,
                   self._composer_nom(self.masque_mas, i),
                   str(self.param['angle_vision']),
                   str(self.param['latitude']),
                   pas_min,
                   self._composerJJMM(i)]

            retcode = subprocess.call(com,
                                    stdout = self.fichier_sortie, 
                                    stderr = self.fichier_sortie)
            self.historique_commande.append(com)
            
        return retcode
                   

    def calculer_surface(self):
        """
        Calculates geometry surfaces

        Calls the external command 'surf_cont'
        """
        com = ['surf_cont', self.scene_cir, self.carac['surface']]
             
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
        
    def calculer_flux_sol_diffus_benoit(self, 
                                        nb_transmission = -1, 
                                        fichier_meteo = None,
                                        execute = True):
        """
        calcul du rayonnement solaire diffus
        
        appelle la fonction externe 'masque_ciel_discret_transparence_meteo'
        
        doivent être renseignés
        ** geom scene
        ** geom masque
        ** ciel + angles solides
        ** liste jours
        """
        chrono = Chrono('calculer_flux_sol_diffus', 'solCommand') 
        for i, jour in enumerate(self.liste_jours):
            print('= calcul jour : %s' % i)
            
            com = ['masque_ciel_discret_transparence_meteo',
                   self.scene_cir,
                   self.masque_cir,
                   self.ciel_cir,
                   self.ciel_angle_solide_val,
                   self._composer_nom(self.luminance_ciel_val,jour),
                   str(self.param['angle_vision']),
                   jour[2],
                   jour[3],
                   jour[4],
                   self._composer_nom(self.var['flux_sol_diffus'],i)]

            if nb_transmission != 0:
                com += ['-o1', self.carac['transmittance']]
                com += ['-o5', str(nb_transmission)]

            if fichier_meteo:
                com += ['-o2', self._composer_nom(self.chemin_meteo_diffus, i)]

            if execute:
                retcode = subprocess.call(com,
                                          stdout = self.fichier_sortie, 
                                          stderr = self.fichier_sortie)
            else:
                retcode = 0
            self.historique_commande.append(com)
            if retcode != 0:
                return retcode

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

    def calculer_eclairements(self, fichier_meteo = None, geo = None):
        if not fichier_meteo:
            self.calculer_luminance_ciel()

        self.calculer_flux_solaires(fichier_meteo = fichier_meteo,geo=geo)
        if not os.path.exists(self.facform+'.fac'):
            self.calculer_fac_form()
        self.calculer_radiosite()
    
    def transitoire(self, suffixe_avant, suffixe_apres, meteo = False):
        """
        lance une simulation de temperature de surface à l'aide de la fonction
        transitoire_h_options2 en utilisant le suffixe du pas de temps précédent
        -suffixe_avant- et le suffixe du pas de temps calculé -suffixe_apres-
        """
        if meteo:
            self.definir_meteo_ts(suffixe_apres)
        
        com = ['transitoire_h_options2',
               str(self.t_meteo),
               self.var['Tse'] + '_tr_'+ suffixe_avant,
               self.var['Tn1'] +'_tr_'+ suffixe_avant,                         
               self.var['Tn2'] + '_tr_'+suffixe_avant,
               self.carac['Tint'],
               self.carac['Tfix'],
               self.var['flux_sol_total'] +'_'+ suffixe_apres,
               self.carac['albedo'],
               self.carac['emissivite'],
               self.carac['no_paroi_tr'],
               self.var['hc'],
               self.carac['h_int'],
               self.carac['surface'],
               self.facform,
               self.carac['ffSky'],
               str(self.pas_de_temps),
               self.paroi_tr,
               self.var['Tse'] + '_tr_'+suffixe_apres,
               self.var['Tn1'] + '_tr_'+suffixe_apres,
               self.var['Tn2'] + '_tr_'+suffixe_apres,
               str(self.opt_interreflexions),
               str(self.flux_ir),
               self.option_calc,
               self.option_resul]

        retcode = subprocess.call(com,
                             stdout = self.fichier_sortie, 
                             stderr = self.fichier_sortie)
        self.historique_commande.append(com)
                             
        return retcode
        
    def transitoire_c(self, meteo = False):
        """
        lance une serie de simulation de calcul de temperature de surface
        sur la periode definie dans liste_ts_sol, en utilisant la fontion
        "transitoire_h_option"
        
        utilise les descripteurs avec le suffixe init pour initialiser le calcul
        """
        
        if len(self.dic_meteo) == len(self.liste_ts_sol) and meteo:
            meteo_bool = True
        liste_ts = self.liste_ts_sol
        chrono = Chrono('transitoire_ts', 'solCommande')
        
        for i, ts in enumerate(liste_ts):
            if i == 0:
                self.transitoire('init', liste_ts[0], meteo = meteo_bool)
            else:
                self.transitoire(liste_ts[i-1], liste_ts[i], meteo = meteo_bool)
                
        chrono.fin()
            
    def _creer_option_resul_tr(self):
        """
        cree le fichier option resul qui est utilisé par transitoire pour 
        définir quelles sont les grandeurs de suivies
        """
        option_resul = 'nom_generique %s\n' % JOIN(self.chemin_transitoire, 
                                                   'opt')
        option_resul += LISTE_OPTION
        
        fichier = open(self.option_resul+'.txt','w')
        fichier.write(option_resul)
        fichier.close()
        
    def _creer_option_resul_ts(self, tout):
        """
        cree le fichier option resul qui est utilisé par simulation_Ts pour 
        définir quelles sont les grandeurs de suivies
        """
        option_resul = ''
        if tout:
            for var in VARIABLES_GLO:
                option_resul += '%s %s\n' % (var, self.var[var])
                fichier = open(self.option_resul+'.txt','w')
                fichier.write(option_resul)
                fichier.close()
        
    def simulation_Ts(self, suffixe_avant, suffixe_apres, meteo = False):
        """
        lance une simulation de température de surface en utilisant la fonciton
        "simulation_Ts" de Julien Bouyer
        
        prends en compte des conditions non homogène pour la température de
        l'air et les echanges convectifs
        """
        if meteo:
            self.definir_meteo_ts(suffixe_apres)
        
        com = ['simulation_Tsurface', # 0
               str(self.pas_de_temps),# 1
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
               self.carac['transmittance'], # 16
               self.carac['evaporation'], # 17
               self.meteo_sol, # 18
               self.var['Tse'] + '_' + suffixe_avant, # 19
               self.var['Tn1'] + '_' + suffixe_avant, # 20
               self.var['Tn2'] + '_' + suffixe_avant, # 21
               self.var['Tse'] + '_' + suffixe_apres, # 22
               self.var['Tn1'] + '_' + suffixe_apres, # 23
               self.var['Tn2'] + '_' + suffixe_apres] # 24
               
        verf = [self.carac['surface']+'.val', # 2
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
               self.carac['transmittance']+'.val', # 16
               self.carac['evaporation']+'.val', # 17
               self.var['Tse'] + '_' + suffixe_avant+'.val', # 19
               self.var['Tn1'] + '_' + suffixe_avant+'.val', # 20
               self.var['Tn2'] + '_' + suffixe_avant+'.val']
               
        for ligne in verf:
            if not os.path.isfile(ligne):
                print(ligne, 'n existe pas')
                break
                
        self.bak.append([time.time(), com])
        
        retcode = subprocess.call(com,
                                  stdout = self.fichier_sortie, 
                                  stderr = self.fichier_sortie)
        self.historique_commande.append(com)                             

        return retcode

    def simulation_Ts_veg(self, suffixe_avant, suffixe_apres, meteo = False):
        """
        lance une simulation de température de surface en utilisant la fonciton
        "simulation_Ts" de Julien Bouyer
        
        prends en compte des conditions non homogène pour la température de
        l'air et les echanges convectifs
        """
        if meteo:
            self.definir_meteo_ts(suffixe_apres)
        
        com = ['simulation_Ts_VEG', # 0
               str(self.pas_de_temps),# 1
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
               self.var['Tse'] + '_' + suffixe_avant, # 19
               self.var['Tn1'] + '_' + suffixe_avant, # 20
               self.var['Tn2'] + '_' + suffixe_avant, # 21
               self.var['Ta'] + '_' + suffixe_avant, # 22
               self.var['Tse_veg'] + '_' + suffixe_avant, # 23
               self.var['Tse'] + '_' + suffixe_apres, # 24
               self.var['Tn1'] + '_' + suffixe_apres, # 25
               self.var['Tn2'] + '_' + suffixe_apres, # 26
               self.var['Ta'] + '_' + suffixe_apres, # 27
               self.var['Tse_veg'] + '_' + suffixe_apres, # 28
               self.coeff_veg, # 29
               self.option_resul] # 30

               
        verf = [self.carac['surface']+'.val', # 2
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
               self.var['Tse'] + '_' + suffixe_avant+'.val', # 19
               self.var['Tn1'] + '_' + suffixe_avant+'.val', # 20
               self.var['Tn2'] + '_' + suffixe_avant+'.val', #21
               self.var['Ta'] + '_' + suffixe_avant+'.val', # 22
               self.var['Tse_veg'] + '_' + suffixe_avant+'.val', # 23
               self.coeff_veg+'.txt', # 29
               self.option_resul+'.txt'] # 30

        for ligne in verf:
            if not os.path.isfile(ligne):
                print(ligne, 'n existe pas')
                break
                
        self.bak.append([time.time(), com])


        retcode = subprocess.call(com,
                                  stdout = self.fichier_sortie, 
                                  stderr = self.fichier_sortie)
        self.historique_commande.append(com)
        
        return retcode


    def simulation_Ts_veg_sens(self,
                               suffixe_avant,
                               suffixe_apres,
                               meteo = False,
                               verbose= False,
                               nom_commande = None,
                               args_plus = None):
        """
        lance une simulation de température de surface en utilisant la fonciton
        "simulation_Ts" de Julien Bouyer
        
        prends en compte des conditions non homogène pour la température de
        l'air et les echanges convectifs
        """
        if meteo:
            self.definir_meteo_ts(suffixe_apres)

        if verbose:
            print(('simulation de %s a %s' % (suffixe_avant, suffixe_apres)))

        if nom_commande:
            nom_com = nom_commande
        else:
            nom_com = 'simulation_Ts_VEG_sens'

        com = [nom_com, # 0
               str(self.pas_de_temps),# 1
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
               self.var['Tse'] + '_' + suffixe_avant, # 19
               self.var['Tn1'] + '_' + suffixe_avant, # 20
               self.var['Tn2'] + '_' + suffixe_avant, # 21
               self.var['Ta'] + '_' + suffixe_avant, # 22
               self.var['Tse_veg'] + '_' + suffixe_avant, # 23
               self.var['Tse'] + '_' + suffixe_apres, # 24
               self.var['Tn1'] + '_' + suffixe_apres, # 25
               self.var['Tn2'] + '_' + suffixe_apres, # 26
               self.var['Ta'] + '_' + suffixe_apres, # 27
               self.var['Tse_veg'] + '_' + suffixe_apres, # 28
               self.coeff_veg, # 29
               self.option_resul, # 30
               self.carac['Cf'], # 31
               self.carac['Ca'], # 32
               self.carac['h_ainf'], # 33
               self.carac['h_pa'], # 34
               self.carac['LAI'], # 35
               self.carac['v']]  # 36

        if args_plus:
            for arg in args_plus:
                com.append(str(arg))
               
        verf = [self.carac['surface']+'.val', # 2
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
                self.var['Tse'] + '_' + suffixe_avant+'.val', # 19
                self.var['Tn1'] + '_' + suffixe_avant+'.val', # 20
                self.var['Tn2'] + '_' + suffixe_avant+'.val', #21
                self.var['Ta'] + '_' + suffixe_avant+'.val', # 22
                self.var['Tse_veg'] + '_' + suffixe_avant+'.val', # 23
                self.coeff_veg+'.txt', # 29
                self.option_resul+'.txt', # 30
                self.carac['Cf'] + '.val', # 31
                self.carac['Ca'] + '.val', # 32
                self.carac['h_ainf'] + '.val', # 33
                self.carac['h_pa'] + '.val', # 34 
                self.carac['LAI']+'.val', # 35
                self.carac['v']+'.val'] # 36

        for ligne in verf:
            if not os.path.isfile(ligne):
                print(ligne, 'n existe pas')
                break
                
        self.bak.append([time.time(), com])
        
        retcode = subprocess.call(com,
                                  stdout = self.fichier_sortie, 
                                  stderr = self.fichier_sortie)
        self.historique_commande.append(com)
        
        return retcode


    def simulation_Ts_c(self, meteo = True):
        """
        lance une serie de simulation de température de surface sur la periode 
        definie dans liste_ts_solen utilisant la foncton "simulation_Ts" 
        de Julien Bouyer
        
        prends en compte des conditions non homogène pour la température de
        l'air et les echanges convectifs
        """
        if len(self.dic_meteo) == len(self.liste_ts_sol) and meteo:
            meteo_bool = True
        
        liste_ts = self.liste_ts_sol
        chrono = Chrono('simulation_ts_c', 'solCommande')
        for i, ts in enumerate(liste_ts):
            if i == 0:
                self.simulation_Ts('init', liste_ts[0], meteo = meteo_bool)
                print('initialisation', liste_ts[0])
            else:
                self.simulation_Ts(liste_ts[i-1], 
                                   liste_ts[i], 
                                   meteo = meteo_bool)
                print('calcul de %s a %s' % (liste_ts[i-1], liste_ts[i]))
        chrono.fin()
        

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
        
    def simulation_Ts_Energie_Bat_veg(self, 
                                      suffixe_avant, 
                                      suffixe_apres, 
                                      meteo = False, 
                                      nom_commande = COM_ENERGIE_BAT_VEG,
                                      verbose = False,
                                      args_plus = None,
                                      terminal = False):

        chrono = Chrono('simulation_Ts_Energie_Bat_veg', 'solCommande', verbose= verbose)
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
               self.var['Ta'] + '_' + suffixe_avant, # 29
               self.var['Tse_veg'] + '_' + suffixe_avant, # 30
               self.var['Tnoeud'] + '_' + suffixe_avant, # 31
               self.var['Tse'] + '_' + suffixe_apres, # 32
               self.var['Tn1'] + '_' + suffixe_apres, # 33
               self.var['Tn2'] + '_' + suffixe_apres, # 34
               self.var['Ta'] + '_' + suffixe_apres, # 35
               self.var['Tse_veg'] + '_' + suffixe_apres, # 36
               self.var['Tnoeud'] + '_' + suffixe_apres, # 37
               self.option_resul,#38
               self.carac['Cf'], # 39
               self.carac['Ca'], # 40
               self.carac['h_ainf'], # 41
               self.carac['h_pa'], # 42 
               self.carac['LAI'], # 43
               self.carac['v']] # 44        

        if args_plus:
            for arg in args_plus:
                com.append(str(arg))
               
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
               self.var['Tse'] + '_' + suffixe_avant + '.val', # 26
               self.var['Tn1'] + '_' + suffixe_avant + '.val', # 27
               self.var['Tn2'] + '_' + suffixe_avant + '.val', # 28
               self.var['Ta'] + '_' + suffixe_avant+'.val', # 29
               self.var['Tse_veg'] + '_' + suffixe_avant+'.val', # 30
               self.var['Tnoeud'] + '_' + suffixe_avant + '.txt', # 31
               self.carac['Cf'] + '.val', # 38
               self.carac['Ca'] + '.val', # 39
               self.carac['h_ainf'] + '.val', # 40
               self.carac['h_pa'] + '.val', # 41 
               self.carac['LAI'] + '.val', # 42
               self.carac['v'] + '.val', # 43        
               self.option_resul + '.txt'] # 44
               
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

        if terminal:
            retcode = subprocess.call(com)
        else:
            retcode = subprocess.call(com,
                                      stdout = self.fichier_sortie, 
                                      stderr = self.fichier_sortie)
        self.historique_commande.append(com)
        self.check_nan(self.var['Tnoeud'] + '_' + suffixe_apres + '.txt')
        chrono.fin()    
        return retcode

    def creer_dic_commande_Ts_EnergieBat(self, 
                                         suffixe_avant, 
                                         suffixe_apres,
                                         simulation_batiment = True,
                                         simulation_vegetation = True):
        """
        creer un fichier de commande pour simulation_Ts_EnergieBat
        """
        dic_com = {
            'simulation_batiment': str(int(simulation_batiment)),
            'simulation_vegetation': str(int(simulation_vegetation)),
            
            'dt' : str(self.pas_de_temps),
            'flux_atm' : str(self.flux_ir),
            'ps_equip' : str(self.param['puissance_sensible']),
            'alpha_arg' : str(self.param['regime_thermique']),
            'saison' : str(self.param['saison']),
            'taux_occup' :  str(self.param['taux_occupation']),
            'T10consigne' : str(self.param['temp_consigne']),

            'paroi' : '%s.txt' % self.paroi,
            'sortie_aux' : '%s.txt' % self.option_resul,
            'sol' : '%s.txt' % self.meteo_sol, 

            'fform' : '%s.fac' % self.facform,

            'classe_paroi' : '%s.val' % self.carac['no_classe'],
            'id_paroi' : '%s.val' % self.carac['no_paroi'],

            'surface' :'%s.val' % self.carac['surface'],
            'fciel' : '%s.val' % self.carac['ffSky'],
            'albedo' : '%s.val' % self.carac['albedo'],
            'transmission' :'%s.val' % self.carac['transmittance_bat'],
            'emissivite' :'%s.val' % self.carac['emissivite'],

            'f_evap_sol' : '%s.val' % self.carac['evaporation'], 
            'LAI' : '%s.val' % self.carac['LAI'],

            'f_h_eau_sol' : '%s.val' % self.carac['h_eau_sol'], 
            'f_dt_eau_sol' : '%s.val' % self.carac['dt_arrosage_sol'], 

            'Cf' : '%s.val' % self.carac['Cf'],
            'Ca' : '%s.val' % self.carac['Ca'],
            'h_ainf' : '%s.val' % self.carac['h_ainf'], 
            'h_pa' : '%s.val' % self.carac['h_pa'],


            'in_air' : '%s.val' % self.carac['entree_air'], 
            'niveau' :  '%s.val' % self.carac['niveau'],

            'flux_sol_dir' : '%s_%s.val' % (self.var['flux_sol_direct'], suffixe_apres), 
            'flux_sol_diff' :  '%s_%s.val' % (self.var['flux_sol_diffus'] , suffixe_apres),
            'ecl_inc_tot' :  '%s_%s.val' % (self.var['ecl_inc_total'] , suffixe_apres),

            'hc_ext' : '%s.val' % self.var['hc'],  
            'Text' : '%s.val' % self.var['Tair'],   
            'w_ext' : '%s.val' % self.var['HR'], 
            'vit' : '%s.val' % self.carac['v'],

            'TSext_init' : '%s_%s.val' % (self.var['Tse'] , suffixe_avant),
            'TPint1_init' : '%s_%s.val' % (self.var['Tp1'] , suffixe_avant),
            'TPint2_init' : '%s_%s.val' % (self.var['Tp2'] , suffixe_avant),
            'TNint1_init' : '%s_%s.val' % (self.var['Tn1'] , suffixe_avant),
            'TNint2_init' : '%s_%s.val' % (self.var['Tn2'], suffixe_avant),
            'TSext' : '%s_%s.val' % (self.var['Tse'], suffixe_apres),
            'TPint1' : '%s_%s.val' % (self.var['Tp1']  , suffixe_apres),
            'TPint2' : '%s_%s.val' % (self.var['Tp2']  , suffixe_apres),
            'TNint1' : '%s_%s.val' % (self.var['Tn1']  , suffixe_apres),
            'TNint2' : '%s_%s.val' % (self.var['Tn2'] , suffixe_apres),

            'TN3_init' : '%s_%s.val' % (self.var['TN3'], suffixe_avant),
            'TN3' : '%s_%s.val' % (self.var['TN3'] , suffixe_apres),
            'TN4_init' : '%s_%s.val' % (self.var['TN4'], suffixe_avant),
            'TN4' : '%s_%s.val' % (self.var['TN4'] , suffixe_apres),
            'TN5_init' : '%s_%s.val' % (self.var['TN5'], suffixe_avant),
            'TN5' : '%s_%s.val' % (self.var['TN5'] , suffixe_apres),
            'TN6_init' : '%s_%s.val' % (self.var['TN6'], suffixe_avant),
            'TN6' : '%s_%s.val' % (self.var['TN6'] , suffixe_apres),
            'TN7_init' : '%s_%s.val' % (self.var['TN7'], suffixe_avant),
            'TN7' : '%s_%s.val' % (self.var['TN7'] , suffixe_apres),
            'TN8_init' : '%s_%s.val' % (self.var['TN8'], suffixe_avant),
            'TN8' : '%s_%s.val' % (self.var['TN8'] , suffixe_apres),
            'TN9_init' : '%s_%s.val' % (self.var['TN9'], suffixe_avant),
            'TN9' : '%s_%s.val' % (self.var['TN9'] , suffixe_apres),
            'TN10_init' : '%s_%s.val' % (self.var['TN10'], suffixe_avant),
            'TN10' : '%s_%s.val' % (self.var['TN10'] , suffixe_apres),
            'TN11_init' : '%s_%s.val' % (self.var['TN11'], suffixe_avant),
            'TN11' : '%s_%s.val' % (self.var['TN11'] , suffixe_apres),
            'TN12_init' : '%s_%s.val' % (self.var['TN12'], suffixe_avant),
            'TN12' : '%s_%s.val' % (self.var['TN12'] , suffixe_apres),
            'TN13_init' : '%s_%s.val' % (self.var['TN13'], suffixe_avant),
            'TN13' : '%s_%s.val' % (self.var['TN13'] , suffixe_apres),
            'TN14_init' : '%s_%s.val' % (self.var['TN14'], suffixe_avant),
            'TN14' : '%s_%s.val' % (self.var['TN14'] , suffixe_apres),
            'TN15_init' : '%s_%s.val' % (self.var['TN15'], suffixe_avant),
            'TN15' : '%s_%s.val' % (self.var['TN15'] , suffixe_apres),
            'TN16_init' : '%s_%s.val' % (self.var['TN16'], suffixe_avant),
            'TN16' : '%s_%s.val' % (self.var['TN16'] , suffixe_apres),
            'TN17_init' : '%s_%s.val' % (self.var['TN17'], suffixe_avant),
            'TN17' : '%s_%s.val' % (self.var['TN17'] , suffixe_apres),
            'TN18_init' : '%s_%s.val' % (self.var['TN18'], suffixe_avant),
            'TN18' : '%s_%s.val' % (self.var['TN18'] , suffixe_apres),
            'TN19_init' : '%s_%s.val' % (self.var['TN19'], suffixe_avant),
            'TN19' : '%s_%s.val' % (self.var['TN19'] , suffixe_apres),
            'TN20_init' : '%s_%s.val' % (self.var['TN20'], suffixe_avant),
            'TN20' : '%s_%s.val' % (self.var['TN20'] , suffixe_apres),
            'TN21_init' : '%s_%s.val' % (self.var['TN21'], suffixe_avant),
            'TN21' : '%s_%s.val' % (self.var['TN21'] , suffixe_apres),

            'h_eau_sol_tot_init' : '%s_%s.val' % (self.var['h_eau_sol_tot'], suffixe_avant),
            'h_eau_sol_tot' : '%s_%s.val' % (self.var['h_eau_sol_tot'] , suffixe_apres),

            'Ta_init' : '%s_%s.val' % (self.var['Ta'] , suffixe_avant),
            'Tse_veg_init' : '%s_%s.val' % (self.var['Tse_veg'] , suffixe_avant),
            'Ta' : '%s_%s.val' % (self.var['Ta'] , suffixe_apres),
            'Tse_veg' : '%s_%s.val' % (self.var['Tse_veg'] , suffixe_apres),
 
            'Tnoeuds_init' : '%s_%s.txt' % (self.var['Tnoeud'], suffixe_avant),
            'Tnoeuds_out' :  '%s_%s.txt' % (self.var['Tnoeud'], suffixe_apres)
            }

        return dic_com

    def ecrire_commande_Ts_EnergieBat(self,
                                      suffixe_avant,
                                      suffixe_apres,
                                      simulation_batiment = True,
                                      simulation_vegetation = True):
        
        dic_com = self.creer_dic_commande_Ts_EnergieBat(suffixe_avant,
                                                        suffixe_apres,
                                                        simulation_batiment,
                                                        simulation_vegetation)
        lst_cle = ['simulation_batiment',
                   'simulation_vegetation',
                   'dt',
                   'flux_atm',
                   
                   'paroi',
                   'sortie_aux',
                   'sol',
                
                   'fform', 
                   
                   'classe_paroi',
                   'id_paroi',
                   
                   'surface',
                   'fciel',
                   'albedo',
                   'transmission',
                   'emissivite',
                   
                   'f_evap_sol',
                   'LAI',

                   'f_h_eau_sol',
                   'f_dt_eau_sol',

                   'flux_sol_dir',
                   'flux_sol_diff',
                   'ecl_inc_tot',

                   'hc_ext',
                   'Text',
                   'w_ext',
                   
                   'TSext_init',
                   'TPint1_init',
                   'TPint2_init',
                   'TNint1_init',
                   'TNint2_init',
                   'TSext',
                   'TPint1',
                   'TPint2',
                   'TNint1',
                   'TNint2', 
                   'TN3_init',
                   'TN3',
                   'TN4_init',
                   'TN4',
                   'TN5_init',
                   'TN5',
                   'TN6_init',
                   'TN6',
                   'TN7_init',
                   'TN7',
                   'TN8_init',
                   'TN8',
                   'TN9_init',
                   'TN9',
                   'TN10_init',
                   'TN10',
                   'TN11_init',
                   'TN11',
                   'TN12_init',
                   'TN12',
                   'TN13_init',
                   'TN13',
                   'TN14_init',
                   'TN14',
                   'TN15_init',
                   'TN15',
                   'TN16_init',
                   'TN16',
                   'TN17_init',
                   'TN17',
                   'TN18_init',
                   'TN18',
                   'TN19_init',
                   'TN19',
                   'TN20_init',
                   'TN20',
                   'TN21_init',
                   'TN21',

                   'h_eau_sol_tot_init',
                   'h_eau_sol_tot']
                   
                   

        if simulation_batiment:
            lst_cle += ['ps_equip',
                        'alpha_arg',
                        'saison',
                        'taux_occup',
                        'T10consigne',
              
                        'in_air',
                        'niveau',
                        
                        'Tnoeuds_init',
                        'Tnoeuds_out']

        if simulation_vegetation:
            lst_cle += ['vit',
                        'Cf',
                        'Ca',
                        'h_ainf',
                        'h_pa',
                        'Ta_init',
                        'Tse_veg_init',
                        'Ta',
                        'Tse_veg']

        txt = ''
        
        for cle in lst_cle :
            txt += '%s\t%s\n' % (cle, dic_com[cle])

        return txt 

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


        txt = self.ecrire_commande_Ts_EnergieBat(suffixe_avant, suffixe_apres, simulation_batiment, simulation_vegetation)

        ecrire_fichier(self.fichier_commande, txt)

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

    def calculer_surface_bonhomme_confort(self):
        com = ['surf_cont',
               self.bonhomme_confort_cir,
               self.bonhomme_confort_surface]

        retcode = subprocess.call(com,
                                  stdout = self.fichier_sortie, 
                                  stderr = self.fichier_sortie)
        self.historique_commande.append(com)

        return retcode


    def calculer_MRT(self, i_ts,
                     terminal = False,
                     verbose = False,
                     nom_commande = None):
        """
        calculer la temperature radiante moyenne
        avec proc_out_mrt
        """

        chrono = Chrono('proc_out_mrt', 'solCommande', verbose= verbose)
        ts = self.liste_ts_sol[i_ts]

        hauteur_soleil = self.info_solaire(i_ts)[1]
        
        if not nom_commande:
            nom_commande = 'proc_out_mrt'

        com = [nom_commande,
               self.bonhomme_confort_cir,
               self.bonhomme_confort_surface,
               self.scene_cir,
               '%s_%s' % (self.var['Tse'], ts),
               self.carac['emissivite'],
               self.masque_cir,
               self.sol_confort_cir,
               '%s_%s' % (self.var_confort['flux_sol_direct'], ts),
               '%s_%s' % (self.var_confort['flux_sol_diffus'], ts),
               self.var_confort['albedo'],
               '%s_%s' % (self.var_confort['vitesse_air'], ts),
               '%s_%s' % (self.var_confort['temperature_air'], ts),
               str(hauteur_soleil),
               self.ciel_cir,
               '%s_%s' % (self.var_confort['MRT'], ts)]

        if terminal:
            retcode = subprocess.call(com)
        else:
            retcode = subprocess.call(com,
                                      stdout = self.fichier_sortie, 
                                      stderr = self.fichier_sortie)

        self.historique_commande.append(com)

        chrono.fin()

        return retcode
