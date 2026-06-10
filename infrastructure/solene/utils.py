#!/usr/bin/python
# -*- coding: utf-8 -*-
"""@package utils
utils 
"""

import time
import numpy as np
import marshal
#import os
        
def text_to_tab(txt):
    """
    converti une suite un tableau de lignes issus d'un fichie texte
    en tableau
    """
    for i in range(len(txt)):
        txt[i] = txt[i].split()

    tab = np.array(txt)
    return tab.transpose()

def ecrire_fichier(file_name, file_chain):
    """
    export the string -string_file- into the file -file_name-
    """
    file = open(file_name, 'w')
    file.write(file_chain)
    file.close()

def norm(vec):
    """
    renvoie la norme du vecteur vec
    """
    return  np.sqrt((vec**2).sum())

def produit_bool(gaa):
    """
    fait le produit booleen en colonne de gaa
    """
    bu = np.ones(len(gaa))
    gaa = gaa.transpose()
    for i in gaa:
        bu = bu * i

    return bu

def tab_distance(point, liste_points):
    """
    renvoie une liste contenant les distances de -point-
    avec chaque point de -liste_points-

    """
    carre = (point-liste_points)**2
    carre = carre.transpose()
    return np.sqrt(carre[0] + carre[1] + carre[2])

def trouver_lien(geo_cible, geo_source):
    """
    renvoi un vecteur contenant les liens de la geometrie cible
    vers la geometrie source
    -> pour chaque triangle de la geometrie cible, c est le lien
    vers le triangle dont le centre de gravite est le plus proche
    dans la geometrie source
    """
    if len(geo_cible.triangles.cdg) == 0:
        geo_cible.calculer_cdg()
    if len(geo_source.triangles.cdg) == 0:
        geo_source.calculer_cdg()
    return trouver_lien_cdg(geo_cible.triangles.cdg, geo_source.triangles.cdg)
#    tab_lien = np.zeros(geo_cible.n_triangles)
#    for i, cdg in enumerate(geo_cible.triangles.cdg):
#        tab_lien[i] = no_min(tab_distance(cdg, geo_source.triangles.cdg))
#    return tab_lien

def trouver_lien_cdg(cdg_cible, cdg_source):
    """
    renvoi un vecteur contenant les liens de la geometrie cible
    vers la geometrie source
    -> pour chaque triangle de la geometrie cible, c est le lien
    vers le triangle dont le centre de gravite est le plus proche
    dans la geometrie source
    """
    tab_lien = np.zeros(len(cdg_cible))
    for i, cdg in enumerate(cdg_cible):
        tab_lien[i] = no_min(tab_distance(cdg, cdg_source))
    return tab_lien

def no_min(lst):
    """
    renvoie l'indice de la valeur la plus faible dans la liste -lst-
    """
    lst2 = np.array(lst)
    return np.arange(len(lst))[lst2 == lst2.min()][0]

def lst2str(lst):
    txt = ''
    for i in lst:
        txt+='%s, '%i
    return txt[:-2]

def couper_ligne_f77(ligne):
    sortie = ''
    n_carac = 75
    for i in range(len(ligne)//n_carac):
        sortie += '&\n&%s'%ligne[:n_carac]
        ligne = ligne[n_carac:]
    sortie += '&\n&%s'%ligne[:n_carac]
    return sortie

def couper_ligne_cpp(ligne, n_carac=75):
    return "".join(ligne[i:i+n_carac] for i in range(0, len(ligne), n_carac))

class Chrono:
    """
    classe permettant de faire des stat sur les temps de calcul 
    """
    def __init__(self, fonction, module):
        self.now = time.time()
        self.fonction = fonction
        self.module = module
        self.duree = 0

    def fin(self):
        """
        imprime la date de fin ainsi que la duree
        """
        end = time.time()
        duree = end-self.now
        if duree > 60:
            duree_str = '%smin %ss' % (int(duree)/60, int(duree)%60)
        else:
            duree_str = str(round(duree, 2))

        self.duree = duree