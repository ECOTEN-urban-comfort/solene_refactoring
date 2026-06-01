#!/usr/bin/python
# -*- coding: utf-8 -*-

# @package solEnv
# le package solEnv est utiliser pour manipuler les fichiers
# utilisés par Solene
#
# en particulier :
#   -création des fichiers de données d'entrée
#          (caractéristiques matériaux, identification des faces...)
#   -recupération des résultats pour une liste
#         de variables données
#
"""@package solEnv
Created on Wed May 19 10:13:24 2010

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr
"""
from numpy import zeros, float32, array, arange, exp, round, ones
import os
import xml.dom.minidom
from shutil import copy

from infrastructure.geometry.legacy.data import Data
from infrastructure.geometry.legacy.timeStep import TimeStep
from infrastructure.geometry.legacy.utils import ecrire_fichier, text_to_tab, no_min, trouver_lien, trouver_lien_cdg
from infrastructure.geometry.legacy.famille import Familles
from infrastructure.geometry.legacy.geom import Geom
from infrastructure.geometry.legacy.solCommand import VARIABLES_GLO
from infrastructure.geometry.legacy.solFile import read_val, write_val, read_cir
from infrastructure.geometry.legacy.vtkFile import VtkFile

lien_num_fam = ['bat', 'solair', 'solbat']

METEO_SOL_DIC = {'lambda_sol': 0.7,
                 'cp_sol': 900,
                 'rho_sol': 1600,
                 'z_ref': 2.0,
                 'i_jour': 196,
                 'i_jour_ref': 1,
                 'moy_Tair': 13.5,
                 'max_Tair': 23.8,
                 'min_Tair': 1.4}

CARAC_VEG = {'a_h_veg': 1,
             'b_h_veg': 1,
             'a_h_ainf': 1}

JOIN = os.path.join


def trouver_niveau(z, lst_niveau):
    z0 = 0
    for i in range(len(lst_niveau)):
        if (z > z0) and (z <= z0 + lst_niveau[i]):
            return i
        else:
            z0 += lst_niveau[i]
    return len(lst_niveau)


def ret_h_ainf(v, L):
    return ret_R(v) * L * 1000 / 3600


def ret_R(v):
    return 125 + (1000 - 125) * v / 10


def ret_h_pa(v, LAI):
    return 2 * LAI * 1200 / ret_re(v)


def ret_re(v):
    return (1174 + 0.1)**0.5 / (207 * v)**0.25


class InterpolVal:
    """
    classe d'interpolation pour passer
    les donnees entre deux geometries/deux maillages
    si type_data == 'val' : utilise les geometries brute
    sinon utlise les centres de gravite
    """

    def __init__(self, geom_cible, geom_source, type_data='val'):
        if type_data == 'val':
            self.geom_cible = geom_cible
            self.geom_source = geom_source
            self.lien = trouver_lien(geom_cible, geom_source).tolist()
        else:
            self.lien = trouver_lien_cdg(geom_cible, geom_source).tolist()

    def interpoler_val(self, fichier_source, fichier_cible):
        val_in = read_val(fichier_source, self.geom_source)
        val_out = val_in[self.lien]
        write_val(fichier_cible, self.geom_cible, val_out)

    def interpoler(self, val_source):
        return val_source[self.lien]


class SolEnv:
    """
    Classe de manipulation des fichiers de solene
    """

    def __init__(self,
                 solCommand,
                 geom_sol,
                 familles=None,
                 data=None,
                 timeStep=None):
        """
        constructeur
        @param solCommand nom et chemins des fichiers solene
        @param geom_sol géométrie solene
        @param familles familles utilisées pour renseigner les caractéristiques
        @param data data contenant les résultats extraits
        @param timeStep liste des pas de temps utilisés
        """

        self.SolCommand = solCommand
        self.geom = geom_sol
        self.geom_sol_confort = Geom()

        if familles:
            self.familles = familles
            self.familles.attribuer_num_familles(self.geom.familles)
        else:
            self.familles = Familles()

        if data:
            self.data = data
        else:
            self.data = Data(geom=geom_sol)

        if TimeStep:
            self.TimeStep = timeStep
        else:
            self.TimeStep = TimeStep()

        self.dic_cas = {}
        self.carac_val = {}

    def definir_meteo_liste(self, liste_meteo):
        """
        importer la meteo sous forme de liste
        """
        self.meteo_liste = liste_meteo

    def recuperer_familles_sol(self):
        """
        recupere des informations sur les familles d'apres les fichiers
        'paroi.txt', 'no_paroi.val' et 'no_classe.val'
        """
        no_paroi = read_val(self.SolCommand.carac['no_paroi'] + '.val',
                            geom=self.geom)
        no_classe = read_val(self.SolCommand.carac['no_classe'] + '.val',
                             geom=self.geom)

    def ecrire_descripteur(self, nom_carac, val, faces=False):
        self.carac_val[nom_carac] = val
        write_val(self.SolCommand.carac[nom_carac],
                  self.geom, val,
                  faces=faces)

    def creer_descripteur_carac(self,
                                nom_carac,
                                nom_descripteur=None,
                                faces=False):
        """
        cree le descripteur -nom_descripteur- pour la caractéristique matériau
        nom_carac
        """

        if faces:
            n_elem = self.geom.n_faces
            type_elem = self.geom.faces
        else:
            n_elem = self.geom.n_triangles
            type_elem = self.geom.triangles

        carac = zeros(n_elem)

        for elem in range(n_elem):
            famille = self.familles.trouver_famille(type_elem.famille[elem])
            if nom_carac in famille.param:
                carac[elem] = famille.param[nom_carac]
            else:
                carac[elem] = 0
        if nom_descripteur:
            write_val(nom_descripteur,
                      geom=self.geom,
                      data=carac,
                      faces=faces)
        else:
            return carac

    def creer_descripteur(self, carac, write_carac=True):
        """
        cree le descripteur pour la caractériqtique -carac- avec le nom defini dans
        solCommand
        """
        if write_carac:
            self.creer_descripteur_carac(carac, self.SolCommand.carac[carac])
        else:
            return self.creer_descripteur_carac(carac)

    def creer_descripteur_constant(self, nom_var, valeur):
        """
        cree un descripteur constant pour la variable nom_var
        """
        data = [valeur] * self.geom.n_triangles
        write_val(self.SolCommand.var[nom_var], self.geom, data)

    def creer_descripteur_solaires(self):
        """
        cree les desecripteurs solaires albedo et transmittance
        """
        alb = self.creer_descripteur('albedo', write_carac=False)
        write_val(self.SolCommand.carac['albedo'], self.geom, alb)
        self.carac_val['albedo'] = alb

        trans = self.creer_descripteur('transmittance', write_carac=False)

        if 'vitrage' in self.familles.renvoyer_classes_presentes(
                list(self.geom.familles.keys())):
            familles_vitrage = self.familles.renvoyer_liste_famille_classes(
                'vitrage', liste_nom=True)
            bool_vitrage = self.geom.selection_famille(
                familles_vitrage, faces=False)

            write_val(
                self.SolCommand.carac['transmittance_bat'],
                self.geom,
                trans)
            self.carac_val['transmittance_bat'] = trans
            trans[bool_vitrage] = 0
            write_val(self.SolCommand.carac['transmittance'], self.geom, trans)
            self.carac_val['transmittance'] = trans
        else:
            write_val(self.SolCommand.carac['transmittance'], self.geom, trans)
            write_val(
                self.SolCommand.carac['transmittance_bat'],
                self.geom,
                trans)
            self.carac_val['transmittance_bat'] = trans
            self.carac_val['transmittance'] = trans

    def creer_descripteur_veg(self):
        """
        creer les descripteur pour les surfaces vegetales
        """
        epaisseur_feuillage = self.creer_descripteur_carac(
            'epaisseur_feuillage')
        coeff_extinction = self.creer_descripteur_carac('coeff_extinction')

        LAI_val = self.creer_descripteur_carac('LAI')
        tau_val = read_val(
            self.SolCommand.carac['transmittance_bat'],
            self.geom)

        Ca_val = 1200 * epaisseur_feuillage
        Cf_val = 2400 * LAI_val

        tau_val[LAI_val != 0] = round(
            exp(-coeff_extinction * LAI_val)[LAI_val != 0], 3)

        write_val(self.SolCommand.carac['Ca'], self.geom, Ca_val)
        write_val(self.SolCommand.carac['Cf'], self.geom, Cf_val)
        write_val(self.SolCommand.carac['LAI'], self.geom, LAI_val)
        write_val(
            self.SolCommand.carac['transmittance_bat'],
            self.geom,
            tau_val)

        self.carac_val['epaisseur_feuillage'] = epaisseur_feuillage
        self.carac_val['coeff_extinction'] = coeff_extinction
        self.carac_val['LAI'] = LAI_val
        self.carac_val['Ca'] = Ca_val
        self.carac_val['Cf'] = Cf_val
        self.carac_val['transmittance_bat'] = tau_val
        self.carac_val['bool_veg'] = LAI_val != 0

    def definir_meteo_veg(self, v):
        """
        cree les descipteurs pour la vegetation dependant des
        conditions meteo (vitesse du vent)
        """
        bool_veg = self.carac_val['bool_veg']
        LAI = self.carac_val['LAI']
        epaisseur_feuillage = self.carac_val['epaisseur_feuillage']

        v_val = v * ones(self.geom.n_triangles)

        # hc_val = (5.7 + 3.8 * v_val)
        hc_val = read_val(self.SolCommand.var['hc'], self.geom)
        hc_val[bool_veg] = (
            CARAC_VEG['a_h_veg'] *
            v_val +
            CARAC_VEG['b_h_veg'])[bool_veg]

        h_pa_val = round(ret_h_pa(v_val * CARAC_VEG['a_h_ainf'], LAI), 3)
        h_ainf_val = round(
            ret_h_ainf(
                v_val *
                CARAC_VEG['a_h_ainf'],
                epaisseur_feuillage),
            3)

        v_val = v * ones(self.geom.n_triangles)

        write_val(self.SolCommand.var['hc'], self.geom, hc_val)
        write_val(self.SolCommand.carac['h_pa'], self.geom, h_pa_val)
        write_val(self.SolCommand.carac['h_ainf'], self.geom, h_ainf_val)
        write_val(self.SolCommand.carac['v'], self.geom, v_val)

    def creer_param_simulation_ts(self, HR=True, T_init=None, hc=None):
        """
        cree les paramètres matériaux pour les simulations de température de
        surface
            emissivite (.val)
            T_int (.val)
            fichier_paroi (.txt)
            numero_paroi (.val)
            meteo_sol (.txt)
            h_eau_sol (.val)
            dt_arrosage_sol (.val)

            plus par defaut:
            HR (.val)
            evaporation (.val)
        """
        self.creer_descripteur('emissivite')
        self.creer_T_int()
        self.creer_fichier_paroi()
        self.creer_numero_paroi()
        self.creer_meteo_sol()
        self.creer_option_resul()
        self.creer_h_eau_sol()
        self.creer_dt_arrosage_sol()
        self.creer_h_eau_sol_init()

        if T_init:
            self.creer_T_init(T_init)

        if hc:
            self.creer_descripteur_constant('hc', hc)

        # nom_trans = self.SolCommand.carac['transmittance']+'.val'
        # self.creer_descripteur_carac('transmittance', nom_trans)

        if HR:
            self.creer_evaporation()
            self.creer_descripteur_constant('HR', 0)

    def creer_param_simulation_ener(
            self,
            n_etages,
            lst_h_niveau=None,
            T_init=None,
            hc=None):
        """
        cree les parametres necessaire pour les simulations energetiques
        (=surfaces+batiment)
        """
        self.creer_param_simulation_ts(T_init=T_init, hc=hc)
        self.creer_fichier_entree_air()
        self.creer_fichier_niveau(n_etages, lst_h_niveau)
        if T_init:
            self.creer_Tnoeud_init(T_init, n_etage=n_etages)

    def definir_HR_meteo(self, i):
        """
        cree un descripteur pour l humidite avec une valeur
        homogene egale a l humidite specifique des donnees meteo
        """
        self.creer_descripteur_constant('HR', self.meteo_liste[i]['w'] / 1000)

    def definir_Tair_meteo(self, i):
        """
        cree un descripteur pour la temperature de l air exterieure
        avec une valeur homogene egale a la temperature
        des donnees meteo
        """
        self.creer_descripteur_constant('Tair', self.meteo_liste[i]['T'])

    def definir_i_jour_sol(self, i):
        """
        cree le fichier meteo pour le sol pour le pas de temps i
        """
        jour = self.TimeStep.jour(i)
        METEO_SOL_DIC['i_jour'] = jour
        self.creer_meteo_sol(dic_meteo=METEO_SOL_DIC)

    def definir_trans_vitrage(
            self,
            i,
            seuil_flux=200,
            trans_ferme=0.3,
            trans_ouvert=0.7):
        """
        definit la transmittivite des vitrage pour un pas de temps i,
        en fonction du flux solaire incident :
            * -seuil_flux- : flux solaire incident a partir
            duquel les strores/volets sont fermes
            * -trans_ferme- : transmittivite stores/volets fermes
            * -trans_ouvert- : tansmittivite stores/volets ouverts
        """
        flux_sol = read_val(
            '%s_%s' %
            (self.SolCommand.var['flux_sol_direct'],
             self.SolCommand.liste_ts_sol[i]),
            self.geom)
        trans = read_val(self.SolCommand.carac['transmittance_bat'], self.geom)
        if 'vitrage' in self.geom.familles:
            lst_vit = arange(
                self.geom.n_triangles)[
                self.geom.triangles.famille == int(
                    self.geom.familles['vitrage'])]
            for i in lst_vit:
                if flux_sol[i] > seuil_flux:
                    trans[i] = trans_ferme
                else:
                    trans[i] = trans_ouvert
        write_val(self.SolCommand.carac['transmittance_bat'], self.geom, trans)

    # ===  h_toit ===#
    def definir_h_toit(self, i):
        """
        definir CHTC d'apres la vitesse sur le toit : relation de Jayamaha
        """
        write_val(
            self.SolCommand.var['hc'],
            self.geom,
            self.meteo_liste[i]['v'] *
            1.444 +
            4.955)

    # ===  h_profil ===#
    def ret_h_profil(self, i):
        """
        renvoi les CHTC d apres la relation de Jayamaha
        en considerant la vitesse variable en hauteur
        d apres un profil puissance
        """
        z = self.geom.triangles.cdg.transpose()[2]
        return ((self.meteo_liste[i]['v'] * (z / 15)
                ** 0.3) * 3.8 + 5.7).round(2)

    def definir_h_profil(self, i):
        """
        cree les CHTC d apres la relation de Jayamaha
        en considerant la vitesse variable en hauteur
        d apres un profil puissance
        """
        hc = self.ret_h_profil(i)
        write_val(self.SolCommand.var['hc'], self.geom, hc)

    # === h_sat ===#
    def definir_cas_vent(self, lst_v, lst_dir, chemin):
        """
        definit les conditions de vent initialisees avec saturne
            * -lst_v- : liste des vitesses calculees
            * -lst_dir- : liste des directions de vent
            * -chemin- : chemin vers le repertoire ou les donnees sont strockees
        """
        self.lst_v = array(lst_v)
        self.lst_dir = array(lst_dir)
        self.chemin_vent = chemin

    def trouver_plus_proche(self, i):
        """
        trouve la combinaison vitesse de vent/direction
        pour laquelle le vent a ete initialise la plus proche des
        conditions meteorologiques reelles du pas de temps i
        """
        dic_met = self.meteo_liste[i]
        v = float(self.lst_v[no_min(abs(dic_met['v'] - self.lst_v))])
        d = int(self.lst_dir[no_min(abs(dic_met['direction'] - self.lst_dir))])
        return v, d

    def ret_v_sat(self, i, val_min=0.2):
        v, d = self.trouver_plus_proche(i)
        v_val = read_val(
            '%s/%s_%s/vitesse.val' %
            (self.chemin_vent, v, d), self.geom)
        v_val[v_val < 0.2] = 0.2
        return v_val.round(2)

    def definir_h_sat(self, i):
        write_val(
            self.SolCommand.var['hc'],
            self.geom,
            self.ret_v_sat(i) * 3.8 + 5.7)

    # ======#
    def definir_meteo(self, i, def_vit=True, def_h=None, veg=False):
        self.definir_HR_meteo(i)
        self.definir_Tair_meteo(i)
        self.definir_i_jour_sol(i)
        if def_vit:
            self.definir_trans_vitrage(i)
        if def_h:
            def_h(i)
        if veg:
            v = self.meteo_liste[i]['v']
            self.definir_meteo_veg(v)

    def creer_meteo_sol(self, dic_meteo=None):
        """
        cree le fichier meteo_sol d'apres les valeurs par defaut ou en fonction
        d'un dictionnaire au meme format
        """

        meteo_sol_dic = {'lambda_sol': 0.7,
                         'cp_sol': 900,
                         'rho_sol': 1600,
                         'z_ref': 2.0,
                         'i_jour': 196,
                         'i_jour_ref': 1,
                         'moy_Tair': 13.5,
                         'max_Tair': 23.8,
                         'min_Tair': 1.4}

        liste_meteo_sol_dic = ['lambda_sol',
                               'cp_sol',
                               'rho_sol',
                               'z_ref',
                               'i_jour',
                               'i_jour_ref',
                               'moy_Tair',
                               'max_Tair',
                               'min_Tair']

        if dic_meteo:
            meteo_sol_dic = dic_meteo
        texte = ''

        for cle in liste_meteo_sol_dic:
            texte += '%s %s\n' % (cle, meteo_sol_dic[cle])

        ecrire_fichier(self.SolCommand.meteo_sol + '.txt', texte)

    def creer_h_int(self, h_sol=10000, h_murs=3):
        """
        crée le descripteur contenant le coefficient d'échange surfacique sur
        la face intérieure des murs et à la face inférieure des sols

        par défaut : h_int est très grand pour les sols et égal à 3 pour les
        murs
        """

        h_int = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            if famille.classe == 'sol':
                h_int.append(h_sol)
            elif famille.classe == 'surface_arbre':
                h_int.append(h_murs)
            else:
                try:
                    h_int.append(famille.param['h_int'])
                except BaseException:
                    print('probleme h_int facette :', i)
                    break

        write_val(name=self.SolCommand.carac['h_int'],
                  geom=self.geom,
                  data=h_int)

    def creer_T_int(self, Tint=20):
        """
        crée le descripteur représentant la température intérieure
        """
        write_val(name=self.SolCommand.carac['Tint'],
                  geom=self.geom,
                  data=Tint)

    def creer_T_init(self, Tinit):
        """
        crée un descripteur avec un suffixe 'init' pour les températures initiales
        à la température Tinit
        """
        for Tnoeud in [
            'Tse',
            'Tp1',
            'Tp2',
            'Tn1',
            'Tn2',
            'Ta',
            'Tse_veg',
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
                'TN21']:
            write_val(self.SolCommand.var[Tnoeud] + '_init', self.geom, Tinit)

    def creer_hc_init(self, valeur=20):
        """
        cree un descripteur hc avec une valeur constante (20 par défaut)
        dans le cas où les valeurs ne sont pas importées de simulations CFD
        """
        self.creer_descripteur_constant('hc', valeur)

    def creer_Tair_init(self, valeur=20):
        """
        cree un descripteur Tair avec une valeur constante (20 par défaut)
        dans le cas où les valeurs ne sont pas importées de simulations CFD
        """
        self.creer_descripteur_constant('Tair', valeur)

    def creer_evaporation(self):
        evapo = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            try:
                evapo.append(famille.param['evaporation'])
            except BaseException:
                evapo.append(0)
        write_val(name=self.SolCommand.carac['evaporation'],
                  geom=self.geom,
                  data=evapo)

    def creer_h_eau_sol(self):
        h_eau = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            try:
                h_eau.append(famille.param['h_eau_sol'])
            except BaseException:
                h_eau.append(0)
        write_val(name=self.SolCommand.carac['h_eau_sol'],
                  geom=self.geom,
                  data=h_eau)

    def creer_dt_arrosage_sol(self):
        dt_arrosage = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            try:
                dt_arrosage.append(famille.param['dt_arrosage_sol'])
            except BaseException:
                dt_arrosage.append(0)
        write_val(name=self.SolCommand.carac['dt_arrosage_sol'],
                  geom=self.geom,
                  data=dt_arrosage)

    def creer_h_eau_sol_init(self):
        """
        crée un descripteur avec un suffixe 'init' pour les hauteurs d'eau
        """
        write_val(name=self.SolCommand.var['h_eau_sol_tot'] + '_init',
                  geom=self.geom,
                  data=0.)

    def creer_numero_paroi(self):
        no_paroi = []
        no_classe = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            try:
                no_paroi.append(famille.num_sol)
                no_classe.append(
                    self.familles.lst_classe.index(
                        famille.classe))
            except BaseException:
                print('famille n %s non trouve' % i)

        write_val(name=self.SolCommand.carac['no_paroi'],
                  geom=self.geom,
                  data=no_paroi)

        write_val(name=self.SolCommand.carac['no_classe'],
                  geom=self.geom,
                  data=no_classe)

    def creer_numero_paroi_tr(self):
        descri = []
        for i in self.geom.triangles.famille:
            famille = self.familles.trouver_famille(i)
            try:
                descri.append(lien_num_fam.index(famille.nom))
            except BaseException:
                descri.append(1)

        write_val(name=self.SolCommand.carac['no_paroi_tr'],
                  geom=self.geom,
                  data=descri)

    def creer_T_fix(self):
        descri = zeros(self.geom.n_triangles)

        for i in range(len(self.geom.triangles.famille)):
            fam = self.familles.trouver_famille(self.geom.triangles.famille[i])
            if fam.classe == 'surface_arbre':
                descri[i] = 1

        write_val(name=self.SolCommand.carac['Tfix'],
                  geom=self.geom,
                  data=descri)

    def creer_fichier_paroi_tr(self):
        chaine_fichier = self.familles.exporter_paroi_transitoire(lien_num_fam)
        ecrire_fichier(
            self.SolCommand.paroi_transitoire +
            '.txt',
            chaine_fichier)

    def creer_fichier_paroi(self):
        chaine_fichier = self.familles.exporter_paroi_simulation_ts()
        ecrire_fichier(self.SolCommand.paroi + '.txt', chaine_fichier)

    def creer_fichier_niveau(self, n_niveau, lst_h_niveau=None):
        """
        cree le fichier niveau en fontion du nombre de niveau si les hauteurs d'etage sont constants
        ou en utilisant une liste de hauteur d'etage (lst_h_niv de longueur egale a n_niveau)

        dans le fichier niveau, les etages sont numerotes :
            * -1 pour les surfaces n appartenant pas au batiment simule
            * 0 pour le rez de chaussee
            * n pour le n-ieme etage
            * n_niveau-1 pour le dernier etage et le toit
        """
        z = self.geom.triangles.cdg.transpose()[2]
        h_niv = z.max() / n_niveau
        niveau = zeros(self.geom.n_triangles)
        lst_fam_bat = self.familles.exporter_liste_familles_carac(
            'energetique')
        lst_num_fam = []

        for nom_fam in lst_fam_bat:
            if nom_fam in self.geom.familles:
                lst_num_fam.append(int(self.geom.familles[nom_fam]))

        for i in range(len(z)):
            no_fam = self.geom.triangles.famille[i]
            if no_fam in lst_num_fam:
                if lst_h_niveau:
                    niveau[i] = trouver_niveau(z[i], lst_h_niveau)
                else:
                    niveau[i] = int(z[i] / h_niv)
                    if niveau[i] == n_niveau:
                        niveau[i] -= 1
            else:
                niveau[i] = -1

        write_val(self.SolCommand.carac['niveau'], self.geom, niveau)

    def creer_fichier_entree_air(self):
        liste_fam_bat = self.familles.exporter_liste_familles_carac(
            'energetique')
        entree_air = zeros(self.geom.n_triangles)
        fam = self.geom.triangles.famille

        for nom_fam in liste_fam_bat:
            if nom_fam in self.geom.familles:
                no_fam = int(self.geom.familles[nom_fam])
                entree_air[fam == no_fam] += 1

        if entree_air.max() > 1:
            print('erreur creation entee_air.val')

        write_val(self.SolCommand.carac['entree_air'], self.geom, entree_air)

    def recuperer_resultat(self, lst_var=None, liste_suffixe=None):
        """
        crée un dictionnaire data[ts]['var'][num_tri] où ts est en entier indiquant le numéro du pas de temps,
        var est une chaine de caractère renseignant le nom de la variable et num_tri est le numéro du triangle.
        Le paramètre lst_var doit être une liste de chaine de caractère
        Le paramètre liste_suffixe est le numéro du pas de temps(??)
        """
        if lst_var is None:
            lst_var = self.choisir_variable()

        if not liste_suffixe:
            liste_suffixe = self.TimeStep.liste_ts_sol

        for i in range(len(liste_suffixe)):
            for variable in lst_var:
                nom_val = '%s_%s.val' % (self.SolCommand.var[variable],
                                         liste_suffixe[i])
                donnee = read_val(nom_val, geom=self.geom)
                self.data.ajouter_donnee(i, donnee, variable)

    def exporter_vtu(self, nom, lst_var=None, lst_suff=None):
        if not lst_suff:
            liste_suffixe = self.TimeStep.liste_ts_sol
        else:
            liste_suffixe = lst_suff

        if not lst_var:
            lst_var = self.choisir_variable()

        self.recuperer_resultat(lst_var, liste_suffixe=liste_suffixe)

        for i in range(len(liste_suffixe)):
            vtk = VtkFile(nom + str(i) + '.vtu', geom=self.geom)
            for variable in lst_var:
                vtk.ajouter_donnee(self.data.data[i][variable], variable)
            vtk.close_xml()

    def exporter_vtu_sous_geom(
            self,
            nom,
            sous_geom,
            bool_geom,
            lst_var=None,
            lst_suff=None):
        if not lst_suff:
            liste_suffixe = self.TimeStep.liste_ts_sol
        else:
            liste_suffixe = lst_suff

        if not lst_var:
            lst_var = self.choisir_variable()

        self.recuperer_resultat(lst_var, liste_suffixe=liste_suffixe)

        for i in range(len(liste_suffixe)):
            vtk = VtkFile(nom + str(i) + '.vtu', geom=sous_geom)
            for variable in lst_var:
                vtk.ajouter_donnee(
                    self.data.data[i][variable][bool_geom], variable)
            vtk.close_xml()

    def choisir_variable(self):
        liste_suffixe = self.TimeStep.liste_ts_sol

        lst_var_calculee = []
        lst_var = []
        print('\t --> liste des variables calculée:')
        i = 1
        lst_var_calc = list(self.SolCommand.var.keys())
        lst_var_calc.sort()
        for cle_var in lst_var_calc:
            var = self.SolCommand.var[cle_var]
            ok = 1
            for suff in liste_suffixe:
                nom_fichier = '%s_%s.val' % (var, suff)
                if not os.path.isfile(nom_fichier):
                    ok = 0
                    pass

            if ok:
                lst_var_calculee.append(cle_var)
                print('\t\t [%s] : %s' % (i, cle_var))
                i += 1

        print('\t\t [t] : tout')
        #print()

        lst_choix = 't'  # raw_input('\t choix des variables :')
        if lst_choix == 't':
            lst_var = lst_var_calculee
        else:
            if ',' in lst_choix:
                lst_choix = lst_choix.split(',')
            else:
                lst_choix = lst_choix.split()

            for i in range(len(lst_choix)):
                choix = lst_choix[i]
                if '-' in choix:
                    choix = choix.split('-')
                    for j in range(int(choix[0]), int(choix[1])):
                        lst_var.append(lst_var_calculee[j - 1])
                else:
                    lst_var.append(lst_var_calculee[int(choix) - 1])

        return lst_var

    def supprimer_nan(self, nom_rep, verbose=False):
        """
        Pour tous les fichiers avec l'extention.val contenus dans le dossier 'nom_rep', on remplace les nan par 0.
        """
        liste_fichier = os.listdir(nom_rep)
        for nom_fichier in liste_fichier:
            if nom_fichier[-4:] == '.val':
                nom_fichier = os.path.join(nom_rep, nom_fichier)
                fichier = open(nom_fichier)
                chaine_fichier = fichier.read()
                if 'nan' in chaine_fichier:
                    chaine_fichier = chaine_fichier.replace('nan', '0')
                    if verbose:
                        print('\t nan in %s' % nom_fichier)
                fichier.close()
                ecrire_fichier(nom_fichier, chaine_fichier)

    def supprimer_nan_old(self, nom_rep):
        liste_fichier = os.listdir(nom_rep)
        for nom_fichier in liste_fichier:
            if nom_fichier[-4:] == '.val':
                nom_fichier = os.path.join(nom_rep, nom_fichier)
                fichier = open(nom_fichier)
                chaine_fichier = fichier.read()
                chaine_fichier = chaine_fichier.replace('nan', '0')
                fichier.close()
                ecrire_fichier(nom_fichier, chaine_fichier)

    def ajouter_f_premiere_face(self, nom_rep, verbose=False):
        """
        un bug de masque_ciel_lum supprime le f devant la reference de la
        premiere face

        """
        liste_fichier = os.listdir(nom_rep)
        for nom_fichier in liste_fichier:
            if nom_fichier[-4:] == '.val':
                nom_fichier = os.path.join(nom_rep, nom_fichier)
                fichier = open(nom_fichier)
                chaine_fichier = fichier.readlines()
                fichier.close()
                if chaine_fichier[1].split()[0] == '1':
                    chaine_fichier[1] = 'f' + chaine_fichier[1]
                    if verbose:
                        print('\t ajout f pour %s' % nom_fichier)
                fichier_sortie = ''
                for ligne in chaine_fichier:
                    fichier_sortie += ligne

                ecrire_fichier(nom_fichier, fichier_sortie)

    def nettoyer(self):
        for var in ['flux_sol_direct', 'flux_sol_diffus']:
            rep = os.path.split(
                self.ajouter_f_premiere_face(
                    self.SolCommand.var[var]))[0]
            self.ajouter_f_premiere_face(rep)
            self.supprimer_nan(rep)

    def creer_Tnoeud_init(self, Tnoeud_init, n_etage=1):
        texte = ''
        for i in range(n_etage):
            texte += 11 * ('%s ' % round(Tnoeud_init, 2))
            texte = texte[:-1]
            texte += '\n'

        ecrire_fichier(self.SolCommand.var['Tnoeud'] + '_init.txt', texte)

    def lire_Tnoeud_1niveau(self):
        texte = ''
        for suff in self.TimeStep.liste_ts_sol:
            nom_fichier = '%s_%s.txt' % (self.SolCommand.var['Tnoeud'], suff)
            fichier = open(nom_fichier)
            ligne = fichier.readline()
            texte += ligne
            fichier.close()

        return texte[:-1]

    def lire_Tnoeud(self, n_niv):
        lst = []
        for i in range(n_niv):
            lst.append([])
        for suff in self.TimeStep.liste_ts_sol:
            nom_fichier = '%s_%s.txt' % (self.SolCommand.var['Tnoeud'], suff)
            fichier = open(nom_fichier)
            for i in range(n_niv):
                ligne = fichier.readline()
                lst[i].append(ligne.split())
                for j in range(len(lst[i][-1])):
                    lst[i][-1][j] = float(lst[i][-1][j])
            fichier.close()

        return lst

    def ret_Tnoeud_col(self, col, n_niv):
        resu = []
        lst = self.lire_Tnoeud(n_niv)
        lst = array(lst)
        for i in range(n_niv):
            resu.append(lst[i].transpose()[col])
        return resu

    def ret_Tint_n(self, n_niv):
        return self.ret_Tnoeud_col(9, n_niv)

    def ret_Psens_n(self, n_niv):
        resu = []
        lst = self.lire_Tnoeud(n_niv)
        lst = array(lst)
        for i in range(n_niv):
            resu.append(lst[i].transpose()[10])
        return resu

    def ret_Tint(self):
        txt = self.lire_Tnoeud_1niveau()
        txt = txt.split('\n')
        tab = text_to_tab(txt)

        return float32(tab[9])

    def creer_descripteur_TUI(self, nom):
        """
        cree un descripteur en demandant les valeurs pour chaque famille
        de facon interactive
        """
        lst_fam_sol = self.familles.exporter_familles_solene()
        lst_val = {}
        for fam in lst_fam_sol:
            lst_val[fam] = float(input("valeur pour %s :\t" % fam))

        val = zeros(self.geom.n_triangles)

        for i in range(self.geom.n_triangles):
            val[i] = lst_val[self.familles.trouver_famille(
                self.geom.triangles.famille[i]).nom]

        write_val(nom, self.geom, val)

        return 1

    def creer_option_resul(self):
        """
        cree le fichier option_resul.txt ou sont renseigne les chemins vers les fichiers de sorties
        auxiliaires
        TODO : remplacer par des valeurs dans le fichier de conf
        """
        txt = ''
        for var in VARIABLES_GLO:
            txt += '%s %s\n' % (var, self.SolCommand.var[var])
        ecrire_fichier(self.SolCommand.option_resul + '.txt', txt)

    def recuperer_var(self, nom_var, nom_cas=None, ts=None, ts_i=None):
        """
        recupere la valeur d un descipteur nom_var avec eventuellement :
            * un sous repertoire -nom_cas- : obsolete
            * un suffixe de pas de temps -ts-
            * un lien vers un suffixe de pas de temps -ts_i-
        """
        if nom_var in list(self.SolCommand.var.keys()):
            ch = self.SolCommand.var[nom_var]
        elif nom_var in list(self.SolCommand.carac.keys()):
            ch = self.SolCommand.carac[nom_var]
        if nom_cas:
            ch = os.path.join(self.SolCommand.chemin_sortie, nom_cas)
            ch = os.path.join(ch, 'sortie')
            ch = os.path.join(ch, nom_var)
            ch = os.path.join(ch, nom_var)
        if ts:
            ch += '_' + ts
        if ts_i:
            ch += '_' + self.SolCommand.liste_ts_sol[i]
        return read_val(ch, self.geom)

    def creer_rep_var(self,
                      var,
                      nouveau_rep,
                      ext='.val',
                      rep_init=None,
                      num_init=0,
                      copier=True):
        if not rep_init:
            rep_init = self.SolCommand.var_bak[var]
        if not os.path.isdir(nouveau_rep):
            os.mkdir(nouveau_rep)
        if copier:
            copy(
                '%s_%s%s' %
                (rep_init,
                 self.SolCommand.liste_ts_sol[num_init],
                 ext),
                nouveau_rep)
        self.SolCommand.var[var] = '%s/%s' % (nouveau_rep, var)

    def changer_repertoire_resultat(
            self,
            nom_simul,
            lst_resul=[
                'flux_cond',
                'flux_conv',
                'GLO_Total_Net',
                'GLO_Scene_Net'],
            lst_var=[
                'Tse',
                'Tnoeud'],
        rep_init=None,
            copier=True):
        """
        change les repertoire pour les resultats de lst_resul et lst_var
        copie les fichiers d'entree
        """
        if nom_simul not in list(self.dic_cas.keys()):
            self.dic_cas[nom_simul] = {'Tint': None}
        nom_rep = JOIN(self.SolCommand.chemin_sortie, nom_simul)
        nom_rep_entree = JOIN(nom_rep, 'entree')
        nom_rep_sortie = JOIN(nom_rep, 'sortie')
        for rep in [nom_rep, nom_rep_entree, nom_rep_sortie]:
            if not os.path.isdir(rep):
                os.mkdir(rep)
        for cle, it in list(self.SolCommand.carac.items()):
            nom_fichier = '%s.val' % it
            if copier:
                if os.path.isfile(nom_fichier) and not os.path.isfile(
                        JOIN(nom_rep_entree, cle)):
                    copy(nom_fichier, nom_rep_entree)
            self.SolCommand.carac[cle] = '%s/%s' % (nom_rep_entree, cle)
        if copier:
            copy(self.SolCommand.paroi + '.txt', nom_rep_entree)
            copy(self.SolCommand.meteo_sol + '.txt', nom_rep_entree)
        self.SolCommand.paroi = os.path.join(
            nom_rep_entree, os.path.split(
                self.SolCommand.paroi)[1])
        self.SolCommand.meteo_sol = os.path.join(
            nom_rep_entree, os.path.split(
                self.SolCommand.meteo_sol)[1])
        self.SolCommand.option_resul = os.path.join(
            nom_rep_entree, os.path.split(
                self.SolCommand.option_resul)[1])

        for var in lst_resul:
            nom_rep_var = JOIN(nom_rep_sortie, var)
            if not os.path.isdir(nom_rep_var):
                os.mkdir(nom_rep_var)
            self.SolCommand.var[var] = JOIN(nom_rep_var, var)
        self.creer_option_resul()

        for var in lst_var:
            nom_rep_var = JOIN(nom_rep_sortie, var)
            if var == 'Tnoeud':
                self.creer_rep_var(var,
                                   nom_rep_var,
                                   ext='.txt',
                                   rep_init=rep_init,
                                   copier=copier)
            else:
                self.creer_rep_var(var,
                                   nom_rep_var,
                                   rep_init=rep_init,
                                   copier=copier)

    def lire_var(self, var, cas=None, ts=None, ts_i=None):
        if var in list(self.SolCommand.var.keys()):
            ch = self.SolCommand.var[var]
        if var in list(self.SolCommand.carac.keys()):
            ch = self.SolCommand.carac[var]
        if cas:
            ch = os.path.join(self.SolCommand.chemin_sortie, cas)
            ch = os.path.join(ch, 'sortie')
            ch = os.path.join(ch, var)
            ch = os.path.join(ch, var)
        if ts:
            ch += '_' + ts
        if ts_i:
            ch += '_' + self.SolCommand.liste_ts_sol[i]
        read_val(ch, self.geom)

    def init_dic_cas(self, lst):
        for i in lst:
            self.dic_cas[i] = {'Tint': None}

    def ret_Tint(self, cas=None):
        Tnoeud_bak = self.SolCommand.var['Tnoeud']
        self.SolCommand.var['Tnoeud'] = '%s/%s/sortie/Tnoeud/Tnoeud' % (
            self.SolCommand.chemin_sortie, cas)
        TT = array(self.ret_Tint_n(5))
        self.SolCommand.var['Tnoeud'] = Tnoeud_bak
        return TT

    def ret_Tint_lst(self, force=False):
        for cle, it in list(self.dic_cas.items()):
            if not it['Tint'] or force:
                it['Tint'] = self.ret_Tint(cle)

    def ret_bat_bool(self):
        self.lst_bat = self.ret_bool_carac('energetique')

    def ret_bool_carac(self, carac, form='triangle'):
        lst_fam = self.familles.exporter_liste_familles_carac(carac)
        lst_fam_num = []
        for i in lst_fam:
            lst_fam_num.append(int(self.geom.familles[i]))
        if form == 'triangle':
            return self.geom.selection_famille(lst_fam_num)[1]
        else:
            return self.geom.selection_famille(lst_fam_num)

    def importer_geom_confort(self):
        self.geom_sol_confort = read_cir(
            self.SolCommand.sol_confort_cir + '.cir')
        interpol_val = InterpolVal(
            self.geom_sol_confort,
            self.geom,
            type_data='val')
        self.interpoler = interpol_val.interpoler_val

    def creer_param_confort(
            self,
            i_ts,
            flux_sol_direct=None,
            flux_sol_diffus=None,
            albedo=None,
            vitesse_air=None,
            temperature_air=None):
        if self.geom_sol_confort.n_triangles == 0:
            self.importer_geom_confort()

        ts = self.SolCommand.liste_ts_sol[i_ts]

        if flux_sol_direct:
            write_val(
                self.SolCommand.var_confort['flux_sol_direct'] +
                '_' +
                ts,
                self.geom_sol_confort,
                flux_sol_direct)
        else:
            self.interpoler(
                self.SolCommand.var['flux_sol_direct'] + '_' + ts,
                self.SolCommand.var_confort['flux_sol_direct'] + '_' + ts)

        if flux_sol_diffus:
            write_val(
                self.SolCommand.var_confort['flux_sol_diffus'] +
                '_' +
                ts,
                self.geom_sol_confort,
                flux_sol_diffus)
        else:
            self.interpoler(
                self.SolCommand.var['flux_sol_diffus'] + '_' + ts,
                self.SolCommand.var_confort['flux_sol_diffus'] + '_' + ts)

        if albedo:
            write_val(
                self.SolCommand.var_confort['albedo'],
                self.geom_sol_confort,
                albedo)

        if vitesse_air is None:
            vitesse_air = self.meteo_liste[i_ts]['v']
        if temperature_air is None:
            temperature_air = self.meteo_liste[i_ts]['T']

        write_val(
            self.SolCommand.var_confort['vitesse_air'] +
            '_' +
            ts,
            self.geom_sol_confort,
            vitesse_air)
        write_val(
            self.SolCommand.var_confort['temperature_air'] +
            '_' +
            ts,
            self.geom_sol_confort,
            temperature_air)
