#!/usr/bin/python
# -*- coding: utf-8 -*-
"""@package utils
utils 
"""

import time
import numpy as np
import marshal
#import os


def lire_fichier_dic(nom_fichier):
    """
    renvoie un dictionnaire contenant les colonnes du tableau dont les cle correspondent
    à la valeur de la premiere ligne

    ex:
    fichier1 contient
    var1 var2 var3
    12.3 13.5 11.0
    14.5 1.61 12.1
    ...

    dir =lire_fichier_dic(fichier1)
    dic['var1'] = [12.3, 14.5, ...]
    dic['var2'] = [13.5, 1.61, ...]
    """
    txt = open(nom_fichier)
    txt = txt.readlines()
    for i in range(len(txt)):
        txt[i] = txt[i].replace(',','.')
    tab = text_to_tab(txt)
    dic = {}
    for i in range(len(tab)):
        tabbis = tab[i].tolist()
        dic[tab[i][0]] = np.float32(tabbis[1:])
    return dic

def add_dic(dic1, dic2):
    """
    fusionne deux dictionnaires
    """
    dic3 = {}
    for key, value in list(dic1.items()):
        dic3[key] = value
    for key, value in list(dic2.items()):
        dic3[key] = value
    return dic3

def add_dics(lst_dic):
    """
    fusionne une liste de dictionnaire
    """
    dic_out = {}
    for dic in lst_dic:
        for key, value in list(dic.items()):
            dic_out[key] = value

    return dic_out
            
def lire_fichier_tab(nom_fichier):
    """
    renvoie une liste de liste, chaque liste correpondant a une colonne
    """
    fichier = open(nom_fichier).readlines()
    fichier = text_to_tab(fichier)
    return fichier
        
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


    
def mymap(x, xa, xb, ya, yb):
    """
    mappe une valeur -x- d'un interval -xa- -xb- 
    vers un interval -ya- -yb- 
    """
    x = float(x)
    xa = float(xa)
    xb = float(xb)
    ya = float(ya)
    yb = float(yb)

    return ((ya-yb)*x + (xa*yb - xb*ya)) / (xa-xb)

def interpol_multiline(z, lst_z, lst_val):
    """
    interpolation multilineaire pour la valeur z en fonction d'une liste de z
    et d'une liste de valeur pour chaque z
    """
    i = 0
    while lst_z[i] < z:
        i+=1
        
    return mymap(z, lst_z[i-1], lst_z[i], lst_val[i-1], lst_val[i])

def interpolation(lst_z_new, lst_z_old, lst_val):
    """
    
    """
    lst_val_out = []
    for i in lst_z_new:
        lst_val_out.append(interpol_multiline(i, lst_z_old, lst_val))
    return lst_val_out

def exporter_dic_tableau(nom_fichier, dic, remplace_point = True):
    """
    exporte un dictionnaire sous forme de tableau
    """
    texte = ''
    liste_cle = list(dic.keys())
    liste_cle.sort()
    liste_n_elem = []
    
    
    for cle in liste_cle:
        texte += str(cle) + '\t'
        liste_n_elem.append(len(dic[cle]))
    texte += '\n'
    
    if len(set(liste_n_elem)) > 1:
        print('attention, les listes ne font pas toutes la meme taille!')
        
    n_elem = max(liste_n_elem)
    
    for i in range(n_elem):
        for cle in liste_cle:
            try:
                texte += str(dic[cle][i]) +'\t'
            except:
                texte += 'none\t'
        texte += '\n'
        
    if remplace_point:
        texte = texte.replace('.', ',')
        
    ecrire_fichier(nom_fichier, texte)

def dic2dic(dic):
    """
    transforme un dictionnaire a deux niveau en un dictioinnaire a un
    seul niveau en concatenant les cles
    """
    nouvo_dico = {}
    for elem in list(dic.items()):
        for elem2 in list(elem[1].items()):
            nouvo_dico['%s_%s' % (elem[0], elem2[0])] = elem2[1]
    return nouvo_dico
    
def ecrire_date(temps):
    """
    ecriture de la date dans un fomalt lisible
    """
    tps = time.localtime(temps)
    return 'le %s/%s à %s:%s' % (tps[2], tps[1], tps[0], tps[3])
    
def print_debut_fonction(fonction, module):
    """
    affiche le declenchement du chronometre
    """
    #print('\n ==> %s start %s.py, %s()' % (time.ctime(), module, fonction))

def print_fin_fonction(fonction, module, duree):
    """
    affiche la fin du chronometre
    """
    #print('\\\ fin %s() à %s, duree %s' % (fonction, time.ctime(), duree))

    
# def get_data(element_xml, nom):
#     """
#     retourne la valeur du champ -nom- 
#     element_xml : element d'un fichier xml dans lequel le champ nom est unique
#     """
#     try:
#         data = element_xml.getElementsByTagName(nom)[0].firstChild.data
#     except:
#         print 'pas de champ', nom
#         data = None
#     return data

def lire_lrv(nom, n):
    """
    importe une liste de dictionnaire depuis un ensemble de fichier
    ecrit avec le module marshal
    """
    lrv = []
    for i in range(n):
        lrv.append(marshal.load(open(nom+str(i))))
    return lrv

def lst2dic(lrvv):
    """
    converti une liste de dictionnaire en un dictionnaire de liste
    """
    lst_p = []
    for i in list(lrvv[0].keys()):
        if (type(lrvv[0][i]) == float):
            lst_p.append(i)
    drv = {}
    for i in lst_p:
        drv[i] = np.zeros(len(lrvv))
    for i in range(len(lrvv)):
        for vvv in lst_p:
            drv[vvv][i] = lrvv[i][vvv]
    return drv

def no_min(lst):
    """
    renvoie l'indice de la valeur la plus faible dans la liste -lst-
    """
    lst2 = np.array(lst)
    return np.arange(len(lst))[lst2 == lst2.min()][0]

def h2j(lst):
    """
    renvoie la liste des sommes journaliere d'une variable donnee au 
    pas de temps horaire
    """
    lst = np.array(lst)
    resu = np.zeros(len(lst)/24)
    for i in range(len(lst)/24):
        resu[i] = lst[i*24:(i+1)*24].sum()
    return resu

def lst2str(lst):
    txt = ''
    for i in lst:
        txt+='%s, '%i
    return txt[:-2]

def lst2str2(lst):
    return " or ".join(lst)

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
    def __init__(self, fonction, module, verbose = True):
        self.now = time.time()
        self.fonction = fonction
        self.module = module
        self.verbose = verbose
        self.duree = 0
        if verbose:
            self.printdebut()

    def printdebut(self):
        """
        imprime la data de debut
        """
        #print_debut_fonction(self.fonction, self.module)
        pass

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
        if duree > 2 and self.verbose:
            print(' %s, duree %ss, f %s ' % (time.ctime(), 
                                             duree_str, 
                                             self.fonction))
        self.duree = duree
