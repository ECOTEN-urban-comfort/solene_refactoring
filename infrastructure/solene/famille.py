# -*- coding: utf-8 -*-
"""@package famille
Created on Tue May 18 18:06:39 2010

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr
    
    
I. DEFINITIONS
    classe : les classes de surfce definissent le type de surface traite, 
        precisant si elles sont utilise par solene ou par saturne, quelles
        caracteristiques il faut renseigner etc.
    famille : les familles sont l attribut de base des surfaces. 
        Elles appartiennennt à une classe et ne diffèrent aux sein 
        d'une même classe que par la valeurs de leurs caractéristiques.

II. CREATION DES FAMILLES

    Les classes sont définies une fois pour toute.

    Les familles sont définies dans un fichier .xml et sont attribuées 
    aux surfaces par l'intermédiaire d'un attribut définit lors de 
    la création de la géométrie, qui peut être présent dans le .med 
    ou dans un .val attaché à la géométrie .cir utilisée.

    Il faut distinguer le moment où l'on définit les familles, 
    ce qui se fait dans le fichier famille.xml et le moment 
    où ses familles sont attribués aux surfaces.

II.a. Cas de l'utilisation d'un med:

    le numéro est extrait de med et est ajouté dans le champ 'num_med' 
    de chaque famille dans le xml
    
II.b. Cas de l'utilisation d'un .val:

    le nom doit être définit dans le .xml et correspondre au numéro 'num_sol'
    attribué dans le .val attribuant les familles.
    
III. DEFINITIONS DES CLASSES
    
III.a. Liste des classes utilisées par Solene et Saturne
    * mur : surfaces verticales opaques des bâtiments dont la consommation 
        est calculée
    * toit : toit des bâtiments dont la concommation est calculée
    * batiments : surfaces verticales et horizontales des bâtiments dont la 
        consommation n'est pas calculée
    * sol : surfaces horizontales pouvant évaporer de l'eau
    * à ajouter : façades et toitures végétales
    
III.b. Liste des classes utilisées par Solene
    * vitrages : surfaces verticales semi-transparentes des bâtiments dont la 
        consommation est calculée
    * plancher_int : planchers interieurs des batiments simules
    * plancher_bas : plancher bas des batiments simules
    * interieurs : murs interieurs des batiments simules
    * surface_arbre : enveloppe des arbres
    
III.c. Liste des classes utilisees par Saturne
    * sat_inlet : paroi verticales de la veine de simulation aeraulique
        saturne defini d'il s'agit d'entrees ou de sorties
    * volume_arbre : volume des arbres, puits de quantite de mvt dans saturne
    * volume_air
    
IV. CARACTERISTIQUES DES CLASSES
    *radiatif  : les surfaces sont prises en compte dans le calcul radiatif
        il faut que les caracteristiques solaires (reflectivite, transmittivite)
        soit renseignées
    * thermiques : les surfaces sont prises en compte dans 
        le calcul thermiques, il faut que les caracteristiques 
        de conductivite (couches : materiaux, epaisseur) soient renseignéss
    * emissivite : les surfaces sont prises en compte dans le calcul des 
        temperatures de surfaces, il faut que l'emissivite soit rensignee
    * a faire : evaporatif, 

    
    
"""

import xml.dom.minidom
from infrastructure.geometry.legacy.xmlFile import get_data
import sys
from copy import deepcopy

# import logging
# logging.basicConfig(filename='myapp.log', level=logging.INFO)
# logger = logging.getLogger(__name__)

CARAC_CLASSE_BASE = {
    "mur": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": True, "latent": False},
    "sol_new": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": False, "latent": True},
    "mur_veg": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": True, "latent": True},
    "vitrage": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": True, "latent": False},
    "toit": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": True, "latent": False},
    "toit_veg": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": True, "latent": True},
    "plancher_int": {"solene": True, "saturne": False, "radiatif": False, "thermique": True, "emissivite": False, "energetique": False, "latent": False},
    "plancher_bas": {"solene": True, "saturne": False, "radiatif": False, "thermique": True, "emissivite": False, "energetique": False, "latent": False},
    "interieur": {"solene": True, "saturne": False, "radiatif": False, "thermique": True, "emissivite": False, "energetique": False, "latent": False},
    "batiments": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": False, "latent": False},
    "paroi_veg": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": False, "latent": True},
    "sol": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": False, "latent": False},
    "sol_veg": {"solene": True, "saturne": True, "radiatif": True, "thermique": True, "emissivite": True, "energetique": False, "latent": True},
    "masque_sol": {"solene": True, "saturne": True, "radiatif": True, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "surface_arbre": {"solene": True, "saturne": True, "radiatif": True, "thermique": False, "emissivite": True, "energetique": False, "latent": False},
    "volume_arbre": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "volume_air": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "sat_wall": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "sat_inlet": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "sat_sym": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
    "enveloppe_bilan": {"solene": False, "saturne": True, "radiatif": False, "thermique": False, "emissivite": False, "energetique": False, "latent": False},
}

SURFACE_MODEL_OVERRIDES = {
    "mixture": {
        "sol": {"latent": False},
    },
    "azam": {
        "sol": {"latent": True},
    },
    "bb5": {
        "sol": {"latent": True},
    },
}

                
# LIEN NUM FAMILLE
#FAM_NUM_TR = ['bat', 'solair', 'solbat']

# repere numero des classes
LIEN_NUM_CLASSE = ['mur', # 0 
                   'vitrage',  # 1
                   'toit',  # 2
                   'plancher_int', # 3 
                   'plancher_bas', # 4
                   'interieur', # 5
                   'batiments', # 6
                   'sol', # 7
                   'surface_arbre', # 8
                   'eau', # 9
                   'paroi_veg', # 10
                   'mur_veg', # 11
                   'toit_veg', # 12
                   'sol_veg',# 13
                   'sol_new',# 14
                   'masque_sol'] 

def get_materiaux_from_xml(famille_xml):
    """
    recupere la bibliotheque des materiaux stockee dans 
    materiaux.xml
    """
    materiaux_xml = famille_xml.getElementsByTagName('materiau')
    materiaux = {}
    for materiau_xml in materiaux_xml:
        materiau = {}
        materiau['nom'] = get_data(materiau_xml, 'nom')
        for champ in ['conductivite', 'capacite_thermique', 'masse_volumique']:
            materiau[champ] = float(get_data(materiau_xml, champ))
        materiaux[materiau['nom']] = materiau
        
    return materiaux
        
    
def get_layers(famille_xml):
    """
    recupere la liste des couches thermiques pour une famille
    """
    layers_xml = famille_xml.getElementsByTagName('layer')
    layers = [[]]*len(layers_xml)
    
    # verifier si une même position ne reviens pas deux fois
    liste_position = []
    for layer_xml in layers_xml:
        position  = int(get_data(layer_xml, 'position'))
        if position in liste_position:
            print('erreurs deux couches ont la même position!')
            break
        else:
            liste_position.append(position)
    
    for layer_xml in layers_xml:
        position = int(get_data(layer_xml, 'position'))
        layers[position-1] = [get_data(layer_xml, 'materiau'), 
                            float(get_data(layer_xml, 'epaisseur'))]
                            
    return layers
    
def importer_familles_xml(nom_fichier, surface_model):
    """
    importation des familles depuis le fichier -nom_fichier- vers un 
    objet Familles
    """
    fam = Familles(surface_model)
    fam.importer_from_xml(nom_fichier)
    
    return fam

class Famille:
    """
    Objet de decription d'une Famille
    """
    def __init__(self, nom = None, classe = None):
        self.param = {}
        if classe:
            self.classe = classe
        if nom:
            self.nom = nom
        self.couches = []
        
    def verifier(self):
        if self.classe == 'facade':
            try:
                type(self.param['albedo']) == float
            except KeyError:
                print('renseigner albedo')

class Familles:
    """
    Classe Familles definissant un ensemble de familles
    
    les familles peuvent renseigner :
    un type de condition aux limites
    des caracteristi
            self.chemin_familles_xml = get_data(self.param_xml, 'chemin_familles_xml')ques materiaux
    """
    def __init__(self, surface_model):
        self.familles = {}
        self.lst_classe = LIEN_NUM_CLASSE
        self.materiaux = {}
        self.caras_classe = self._build_carac_classe(surface_model)

    def _build_carac_classe(surface_model: str) -> dict:
        carac = deepcopy(CARAC_CLASSE_BASE)

        overrides = SURFACE_MODEL_OVERRIDES.get(surface_model, {})
        for classe, values in overrides.items():
            carac[classe].update(values)

        return carac
        
    def remplir(self, liste_nom):
        """
        renseigne la liste des familles dans le dictionaire -familles-
        
        fonction temporaire en attendant le stockage et le renseigment xml
        """
        for famille in liste_nom:
            self.familles[famille.nom] = famille
    
    def attribuer_num_familles(self, dic_fam_num):
        """
        renseigne le numero de chaque famille tel qu'il est indique dans le 
        dictionnaire -dic_fam_num- issue des familles d'un fichier .med
        """
        #print('\t familles.attribuer_num_familles')
        for fam in list(dic_fam_num.keys()):
            try:
                self.familles[fam].num = int(dic_fam_num[fam])
            except KeyError:
                print('\t -> famille non reconnue :', fam)
            
    def trouver_famille(self, num):
        """int(get_data(famille_xml, 'soleneID'))
        renvoi la famille en fontion de son numero .med
        """
        famille = 0
        for fam in list(self.familles.keys()):
            try:
                num_test = self.familles[fam].num
            except AttributeError:
                pass
            else:
                if num_test == num:
                    famille = self.familles[fam]
                    
        if (famille == 0):
            print("pas de famille trouvee pour le numero %s. les numeros med ont ils ete attribue ?" % num)
                                
        return famille

                
    def exporter_paroi_transitoire(self, fam_num_tr):
        """
        renvoi le contenu du fichier -paroi.txt- pour la fonction transitoire
        sous forme d'une chaine
        """
        fichier = ''
        fichier += '%s\n' % len(fam_num_tr)
        for fam in fam_num_tr:
            famille = self.familles[fam]
            fichier += '%s\n' % fam
            fichier += '%s\n' % len(famille.couches)
            
            for couche in famille.couches:
                mat = self.materiaux[couche[0]]
                fichier += '%s %s %s %s\n' % (couche[1],
                                              mat['conductivite'],
                                              mat['capacite_thermique'],
                                              mat['masse_volumique'])
        return fichier
        
    def exporter_paroi_simulation_ts(self):
        """
        renvoi le contenu du fichier -paroi.txt- pour la fonction simulation_ts
        sous forme d'une chaine
        """
        fichier = ''
        no_famille = 0
        for no_classe in range(len(self.lst_classe)):
            for cle in list(self.familles.keys()):
                fam = self.familles[cle]
                if fam.classe == self.lst_classe[no_classe]:
                    fichier += '\n'
                    fichier += '%s %s %s\n' % (cle, no_classe, no_famille)
                    fam.num_sol = no_famille
#                    if fam not in familles_sat:
#                        fam.num = no_famille
                    no_famille += 1
            
                    fichier += '%s\n' % len(fam.couches)
                    for couche in fam.couches:
                        mat = self.materiaux[couche[0]]
                        fichier += '%s %s %s %s %s\n' % (couche[0],
                                                         couche[1],
                                              mat['conductivite'],
                                              mat['capacite_thermique'],
                                              mat['masse_volumique']) 
                                              
        fichier = str(no_famille) + '\n\n' + fichier
        
        return fichier
        
    def importer_from_xml(self, fichier_xml):
        """
        importe les familles depuis le fichier -fichier_xml-
        """

        doc = xml.dom.minidom.parse(fichier_xml)

        liste_familles = doc.getElementsByTagName('famille')
        
        for famille_xml in liste_familles:
            name = get_data(famille_xml, 'name')
            classe = get_data(famille_xml, 'classe')
            self.familles[name] = Famille(nom = name, classe = classe)
            famille = self.familles[name]

            if self.carac_classe[classe]['solene']:
                # famille.num = int(get_data(famille_xml, 'soleneID'))
                # famille.num_sol = int(get_data(famille_xml, 'soleneID'))
                if classe in ('batiments', 'sol', 'sol_veg', 'paroi_veg', 'masque_sol'): # 'masque_sol'
                    famille.num = 0
                    famille.num_sol = 0
                elif classe == 'surface_arbre':
                    famille.num = 9
                    famille.num_sol = 9

                elif classe == 'plancher_int':
                    famille.num = 3
                    famille.num_sol = 3
                elif classe == 'plancher_bas':
                    famille.num = 4
                    famille.num_sol = 4
                elif classe == 'interieur':
                    famille.num = 5
                    famille.num_sol = 5

                else:
                    raise Exception(f"class {classe} is not in famille.py; either remove the family/families \
with this class from famille.xml, or add this class to famille.py (function 'importer_from_xml')")


            if self.carac_classe[classe]['radiatif']:
                famille.param['albedo'] = float(get_data(famille_xml, 'albedo'))
                famille.param['transmittance'] = float(get_data(famille_xml, 'transmittance'))
            if self.carac_classe[classe]['thermique']:
                famille.couches=get_layers(famille_xml)
            if self.carac_classe[classe]['emissivite']:
                famille.param['emissivite'] = float(get_data(famille_xml, 'emissivite'))
            if self.carac_classe[classe]['latent']:
                try:
                    famille.param['LAI'] = float(get_data(famille_xml, 'LAI'))
                except :
                    pass
                try:
                    famille.param['epaisseur_feuillage'] = float(get_data(famille_xml, 'epaisseur_feuillage'))
                except :
                    pass
                try:
                    famille.param['coeff_extinction'] = float(get_data(famille_xml, 'coeff_extinction'))
                except :
                    pass
                try:
                    famille.param['evaporation'] = float(get_data(famille_xml, 'evaporation'))
                except :
                    pass

        #print()
        #print('LECTURE DES FAMILLES')
        #print('\t nom \t classe           solID  alb    trans  emis   n_couches')
        for cle in self.familles:
            famille = self.familles[cle]
            chaine = '\t'
            chaine += '%s\t %s ' % (famille.nom[:7], famille.classe 
                        + ' '*(16-len(famille.classe)) )
            try:
                chaine += str(famille.soleneID) + '\t'
            except:
                chaine += 'no\t'  

            try:
                chaine += str(famille.param['albedo']) + '\t'
            except:
                chaine += 'no\t'   
            
            try:
                chaine += str(famille.param['transmittance']) + '\t'
            except:
                chaine += 'no\t'
                
            try:
                chaine += str(famille.param['emissivite']) + '\t'
            except:
                chaine += 'no\t'
                
            try:
                chaine += str(len(famille.couches)) + '\t'
            except:
                chaine += 'no\t'
                
            #print(chaine)
            
#            i = 0
#            for couche in famille.couches:
#                i+=1
#                print '\t\tcouche n ',i , couche[0], '\t ', couche[1] 
        #print()

    def renvoyer_classes_presentes(self, lst_famille):
        """
        renvoie la liste des classes auquelles appartiennet les familles
        de lst_familles
        """
        lst_classe = []
        for i_fam in lst_famille:
            if  i_fam in list(self.familles.keys()):
                cl = self.familles[i_fam].classe
                if cl not in lst_classe:
                    lst_classe.append(cl)
        return lst_classe
        
                
    def importer_materiaux_from_xml(self, fichier_xml):
        """
        importe la base de donnes materiaux depuis le fichier -fichier_xml-
        au format xml
        """
        doc = xml.dom.minidom.parse(fichier_xml)
        self.materiaux = get_materiaux_from_xml(doc)
        
    def renvoyer_liste_famille_classes(self, 
                                       liste_classe, 
                                       liste_nom = False, 
                                       chaine = False):
        """
        renvoi la liste des familles appartenant à la classe -nom_classe-
        dans le format précisé liste de nom ou chaine
        """
        if type(liste_classe) != list:
            liste_classe = [liste_classe]
            
        liste_famille = []            
        for nom_classe in liste_classe:
            for cle in list(self.familles.keys()):
                famille = self.familles[cle]
                if famille.classe == nom_classe:
                    liste_famille.append(famille)
                
        liste_famille_nom =  []
        for famille in liste_famille:
            liste_famille_nom.append(famille.nom)

        if liste_nom:
            return liste_famille_nom
        elif chaine:
            return str(liste_famille)[1:-1].replace('\'', '')
        else:
            return liste_famille
            
    def renvoyer_liste_classe_carac(self, carac):
        """
        renvoie la liste des classes ayant la caracteristique -carac-
        """
        lst_classe = []
        for classe, dic_classe_carac in list(self.carac_classe.items()):
            if dic_classe_carac[carac] == True:
                lst_classe.append(classe)
        return lst_classe

    def renvoyer_liste_famille_carac(self, carac, nom = False):
        """
        renvoie la liste des familles ayant la caracteristique -carac-
        """
        lst_familles = self.renvoyer_liste_famille_classes(self.renvoyer_liste_classe_carac(carac))
        if nom:
            for i, i_fam in enumerate(lst_familles):
                lst_familles[i] = i_fam.nom
        return lst_familles

    def exporter_dic_sol(self):
        """
        renvoie sous forme de dictionnaire la liste des familles
        appartenant a la classe solene
        """
        dic = {}
        for famille in list(self.familles.keys()):
            print(famille)
            if self.carac_classe[self.familles[famille].classe]['solene']:
                dic[famille] = self.familles[famille].num_sol
            
        return dic

    def exporter_liste_familles_carac(self, carac):
        """
        renvoie la liste des familles dont la classe a la caracteristique
        -carac-
        """
        lst_classe = self.renvoyer_liste_classe_carac(carac)
        lst_famille = self.renvoyer_liste_famille_classes(lst_classe, 
                                                          liste_nom = True)

        return lst_famille
    
    def exporter_familles_saturne_old(self):
        """
        
        """
        dic = {}
        dic['murs'] = ''
        for classe in list(self.carac_classe.keys()):
            if self.carac_classe[classe]['saturne'] and not self.carac_classe[classe]['thermique']:
                dic[classe] = ''
        
        for nom_fam in list(self.familles.keys()):
            fam = self.familles[nom_fam]
            if self.carac_classe[fam.classe]['saturne']:
                if self.carac_classe[fam.classe]['thermique'] :
                    dic['murs'] += ' ' + nom_fam + ','
                else:
                    dic[fam.classe] += ' ' + nom_fam + ','
                    

        for cle in list(dic.keys()):
            if len(dic[cle]) > 1:
                dic[cle]  = dic[cle][1:-1]

        return dic

    def exporter_familles_saturne(self):
        """
        exports a dictionary containing lists of families according to the types
        necessary for Saturne
        """
        sat_familles = {}
        sat_familles['murs'] = []
        for nom_fam in list(self.familles.keys()):
            fam = self.familles[nom_fam]
            if self.carac_classe[fam.classe]['saturne']:
                if self.carac_classe[fam.classe]['thermique'] :
                    sat_familles['murs'].append(nom_fam)

        sat_familles['sat_inlet'] = self.renvoyer_liste_famille_classes('sat_inlet', liste_nom = True)
        
        sat_familles['sat_wall'] = self.renvoyer_liste_famille_classes('sat_wall', liste_nom = True)
        sat_familles['sat_wall'] += self.renvoyer_liste_famille_classes('masque_sol', liste_nom = True)

        sat_familles['sat_sym'] = self.renvoyer_liste_famille_classes('sat_sym', liste_nom = True)

        sat_familles['surface_arbre'] = self.renvoyer_liste_famille_classes('surface_arbre', liste_nom = True)

        sat_familles['volume_arbre'] = self.renvoyer_liste_famille_classes('volume_arbre', liste_nom = True)

        return sat_familles 
        
    def exporter_familles_solene(self):
        """
        renvoie la liste des noms des familles definies pour solene
        """
        lst_fam_solene = []
        for fam in self.familles:
            fam = self.familles[fam]
            if self.carac_classe[fam.classe]['solene']:
                lst_fam_solene.append(fam.nom)
                
        return lst_fam_solene

        # definition des familles pour les subroutines

    def afficher_famille_solene(self):
        """
        affichage des familles definies pour solene
        """
        lst_fam_solene = self.exporter_familles_solene()
        for i in range(len(lst_fam_solene)):
            lst_fam_solene[i] = self.familles[lst_fam_solene[i]]
        
        for classe in LIEN_NUM_CLASSE:
            for fam in lst_fam_solene:
                if fam.classe == classe:
                    print('%s\t  %s\t  %s' % (LIEN_NUM_CLASSE.index(classe), 
                                              fam.nom, fam.classe))

    def calcul_RC_fam(self, nom_fam):
        """
        Calcule les caracteristiques thermiques d'un famille
        telles qu'estimees dans simulation_ts
        """
        lst_couches = self.familles[nom_fam].couches
        n_couches = len(lst_couches)
        
        R = [0] * n_couches
        Req = 0
        
        for j in range(n_couches):
            R[j] = lst_couches[j][1] / self.materiaux[lst_couches[j][0]]['conductivite']
            Req += R[j]
        
        beta_j = 0
        Rj = 0
        Rj_1 = 0
        Ci_eq = 0
        Ce_eq = 0
        
        for j in range(n_couches):
            mat = self.materiaux[lst_couches[j][0]]
            Rj_1 = Rj
            Rj += R[j]
            beta_j = 0.5 * (Rj + Rj_1) / Req

            Ci_eq += mat['masse_volumique'] * mat['capacite_thermique'] * lst_couches[j][1] * beta_j
            Ce_eq += mat['masse_volumique'] * mat['capacite_thermique'] * lst_couches[j][1] * (1-beta_j)

        self.familles[nom_fam].R = Req
        self.familles[nom_fam].Ci = Ci_eq
        self.familles[nom_fam].Ce = Ce_eq

    def calcul_RC_solene(self):
        """
        pour chaque famille, calcule les caracteristiques thermiques 
        telles qu'estimees dans simulation_ts
        """
        lst_fam_solene = self.exporter_familles_solene()
        for fam in lst_fam_solene:
            self.calcul_RC_fam(fam)
