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

from data import Data
from timeStep import TimeStep
from utils import ecrire_fichier, text_to_tab, no_min
from utils import trouver_lien, trouver_lien_cdg
from famille import Familles
from geom import Geom
from solCommand import VARIABLES_GLO
from solFile import read_val, write_val, read_cir
from vtkFile import VtkFile

lien_num_fam = ['bat', 'solair', 'solbat']

_METEO_SOL_KEYS = (
    "lambda_sol",
    "cp_sol",
    "rho_sol",
    "z_ref",
    "i_jour",
    "i_jour_ref",
    "moy_Tair",
    "max_Tair",
    "min_Tair",
)

CARAC_VEG = {'a_h_veg': 1,
             'b_h_veg': 1,
             'a_h_ainf': 1}

JOIN = os.path.join



def ret_h_ainf(v, L):
    return ret_R(v) * L * 1000 / 3600


def ret_R(v):
    return 125 + (1000 - 125) * v / 10


def ret_h_pa(v, LAI):
    return 2 * LAI * 1200 / ret_re(v)


def ret_re(v):
    return (1174 + 0.1)**0.5 / (207 * v)**0.25


class SolEnv:
    """
    Classe de manipulation des fichiers de solene
    """

    def __init__(self,
                 solCommand,
                 geom_sol,
                 surface_model,
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
            self.familles = Familles(surface_model)

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

    def _definir_meteo_veg(self, v):
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
        self.creer_descripteur("emissivite")
        self._creer_T_int()
        self._creer_fichier_paroi()
        self._creer_numero_paroi()
        self._creer_meteo_sol()
        self._creer_option_resul()

        if self.profile.enable_water_descriptors:
            self._creer_h_eau_sol()
            self._creer_dt_arrosage_sol()
            self._creer_h_eau_sol_init()

        if T_init:
            self._creer_T_init(T_init)

        if hc:
            self.creer_descripteur_constant("hc", hc)

        if HR:
            self.creer_evaporation()
            self.creer_descripteur_constant("HR", 0)

    def _iter_t_init_variable_names(self):
        names = ["Tse"]

        if self.profile.enable_tp_outputs:
            names.extend(["Tp1", "Tp2"])

        names.extend(["Tn1", "Tn2", "Ta", "Tse_veg"])

        if self.profile.enable_extra_transient_nodes:
            names.extend([f"TN{i}" for i in range(3, 22)])

        return names


    def _creer_T_init(self, Tinit):
        for var_name in self._iter_t_init_variable_names():
            write_val(self.SolCommand.var[var_name] + "_init", self.geom, Tinit)

    def _definir_HR_meteo(self, i):
        """
        cree un descripteur pour l humidite avec une valeur
        homogene egale a l humidite specifique des donnees meteo
        """
        self.creer_descripteur_constant('HR', self.meteo_liste[i]['w'] / 1000)

    def _definir_Tair_meteo(self, i):
        """
        cree un descripteur pour la temperature de l air exterieure
        avec une valeur homogene egale a la temperature
        des donnees meteo
        """
        self.creer_descripteur_constant('Tair', self.meteo_liste[i]['T'])

    def _definir_i_jour_sol(self, i):
        jour = self.TimeStep.jour(i)
        self._creer_meteo_sol(dic_meteo={"i_jour": jour})

    def _creer_meteo_sol(self, dic_meteo=None):
        meteo_sol_dic = dict(self.profile.meteo_sol_file_defaults)

        if dic_meteo:
            meteo_sol_dic.update(dic_meteo)

        texte = ""
        for cle in _METEO_SOL_KEYS:
            texte += f"{cle} {meteo_sol_dic[cle]}\n"

        ecrire_fichier(self.SolCommand.meteo_sol + ".txt", texte)

    def _definir_trans_vitrage(
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

    # ======#
    def definir_meteo(self, i, def_vit=True, def_h=None, veg=False):
        self._definir_HR_meteo(i)
        self._definir_Tair_meteo(i)
        self._definir_i_jour_sol(i)
        if def_vit:
            self._definir_trans_vitrage(i)
        if def_h:
            def_h(i)
        if veg:
            v = self.meteo_liste[i]['v']
            self._definir_meteo_veg(v)

    def _creer_T_int(self, Tint=20):
        """
        crée le descripteur représentant la température intérieure
        """
        write_val(name=self.SolCommand.carac['Tint'],
                  geom=self.geom,
                  data=Tint)

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

    def _creer_h_eau_sol(self):
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

    def _creer_dt_arrosage_sol(self):
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

    def _creer_h_eau_sol_init(self):
        """
        crée un descripteur avec un suffixe 'init' pour les hauteurs d'eau
        """
        write_val(name=self.SolCommand.var['h_eau_sol_tot'] + '_init',
                  geom=self.geom,
                  data=0.)

    def _creer_numero_paroi(self):
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

    def _creer_fichier_paroi(self):
        chaine_fichier = self.familles.exporter_paroi_simulation_ts()
        ecrire_fichier(self.SolCommand.paroi + '.txt', chaine_fichier)

    def recuperer_resultat(self, lst_var=None, liste_suffixe=None):
        """
        crée un dictionnaire data[ts]['var'][num_tri] où ts est en entier indiquant le numéro du pas de temps,
        var est une chaine de caractère renseignant le nom de la variable et num_tri est le numéro du triangle.
        Le paramètre lst_var doit être une liste de chaine de caractère
        Le paramètre liste_suffixe est le numéro du pas de temps(??)
        """
        if lst_var is None:
            lst_var = self._choisir_variable()

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
            lst_var = self._choisir_variable()

        self.recuperer_resultat(lst_var, liste_suffixe=liste_suffixe)

        for i in range(len(liste_suffixe)):
            vtk = VtkFile(nom + str(i) + '.vtu', geom=self.geom)
            for variable in lst_var:
                vtk.ajouter_donnee(self.data.data[i][variable], variable)
            vtk.close_xml()

    def _choisir_variable(self):
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
        print()

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

    def _creer_option_resul(self):
        """
        cree le fichier option_resul.txt ou sont renseigne les chemins vers les fichiers de sorties
        auxiliaires
        TODO : remplacer par des valeurs dans le fichier de conf
        """
        txt = ''
        for var in VARIABLES_GLO:
            txt += '%s %s\n' % (var, self.SolCommand.var[var])
        ecrire_fichier(self.SolCommand.option_resul + '.txt', txt)