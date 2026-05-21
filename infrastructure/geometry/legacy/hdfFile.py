#!/usr/bin/python
# -*- coding: utf-8 -*-

"""@package hdfFile
Définition des classes HdfFile, MedFile et CplFile

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr

"""
import numpy as np

import tables
import os

from infrastructure.geometry.legacy.utils import Chrono
from infrastructure.geometry.legacy.geom import *


parallel_domain = 'CHA/parallel domain/MAI.TE4/                  -1                  -1/Fluid volume/CO'


def array_to_table(vec, n):
    """ transforme les array de dim1 issues des .med en array de dim2
    avec des vecteurs contenant 'n' valeurs
    """

    table = vec.reshape(n, len(vec) // n).transpose()
    return table


def table_to_array(table):
    """
    transforme les tables de shape (n_tetras, nPoints)
    en array de dimension1
    """

    if type(table) == np.ndarray:
        a = table.transpose().flatten()

    return a


def liste_to_arrays(liste):
    """
    cree un dataset compose de d'un array 'valeur' et d'un array 'offset'
    pour stocker les elements d'une liste de liste longueur variable

    ex :
        liste   = [[1,2,3], [4,5], [6,7,8]]
            donne :
        valeurs = [1,2,3,4,5,6,7,8]
        regle   = [3,2,3]
    """
    regle = np.zeros(len(liste))
    valeurs = []

    for i in range(len(liste)):
        regle[i] = len(liste[i])
        if regle[i] == 0:
            valeurs.append(0)
        else:
            for j in liste[i]:
                valeurs.append(j)

    valeurs = np.array(valeurs)
    return regle, valeurs


def arrays_to_liste(regle, valeur):
    """
    renvoi une liste de listes avec un nombre d'elements variables en fonction
    d'un array 'valeur' contenant les valeurs et d'un array 'offset' contenant
    les regles pour repartir ces valeurs entre les elements
    """
    liste = []
    index = 0
    for i in range(len(regle)):
        ligne = []
        for j in range(int(regle[i])):
            ligne.append(valeur[index])
            index += 1
        liste.append(ligne)

    return liste


def tab_to_str(tab_nom):
    cpt = 0
    nom = ''
    while tab_nom[cpt] != ' ':
        nom += tab_nom[cpt]
        cpt += 1
    return nom


class HdfFile:
    """
    Classe pour manipuler les fichier de type .hdf : .med (salome, saturne)
    et .cpl (format peronnel)
    """

    def __init__(self, nom, geom=None):
        self.nom = nom
        self.fichier_existe = False
        if geom:
            self.geom = geom

        if os.path.isfile(nom):
            self.fichier_existe = True
            self.hdf = tables.open_file(nom, 'a')
        else:
            self.hdf = tables.open_file(nom, 'w')

    def _path_to_table(self, chemin, n):
        """
        recupere l'array du noeud -chemin- et le transforme en table de dim2
        contenant n valeurs
        """
        noeud = self.hdf.get_node(chemin).read()
        
        vec = noeud
        
        #     transforms the dim1 arrays from the .med files into dim2 arrays
        #     with vectors containing 'n' values   
        
        table = vec.reshape(n, len(vec) // n).transpose()
    
        return table        
        
        #return array_to_table(noeud, n)


class MedFile(HdfFile):
    """
    Classe de manipulation des fichiers .med cree par salome ou gmsh et utilise
    par Saturne
    """

    def __init__(self, nom):
        self.nom = nom
        self.hdf = tables.open_file(nom)

        self.points = ''
        
        self.pointsTE4 = ''
        self.familleTE4 = ''
        
        self.pointsTR3 = ''
        self.familleTR3 = ''
        
        self.pointsSE2 = ''
        self.familleSE2 =  ''
        
        self.list_of_famille = ''
        
        self.extraire_version()

    def extraire_version(self):
        infos_generales = self.hdf.get_node('/INFOS_GENERALES')
        self.med_version_maj = self.hdf.get_node_attr(infos_generales, 'MAJ')
        self.med_version_min = self.hdf.get_node_attr(infos_generales, 'MIN')
        self.med_version_rel = self.hdf.get_node_attr(infos_generales, 'REL')
        self.med_version = [self.med_version_maj, self.med_version_min, self.med_version_rel]

    def extraire_geom(self):
        """
        Extract the geometry from the MED file.
        Retrieve information about edges, triangles, tetrahedra,
        and their respective families.
        """

        chrono = Chrono('extraire_geom', 'hdfFile')

        self.extraire_chemins()
        
        print(f"extract path works and the points path is {self.points} ")
        
        if len(self.hdf.list_nodes('/ENS_MAA')) == 1:
            print('\t this file is indeed a mesh file\n')

            self.geom = Geom()
            self.geom.points = self._path_to_table(self.points, 3)

            self.geom.n_points = len(self.geom.points)

            try:
                self.geom.tetras.points = self._path_to_table(self.pointsTE4, 4)
                self.geom.tetras.famille = self.hdf.get_node(self.familleTE4).read()
            except BaseException:
                self.geom.tetras.points = np.array([])
                self.geom.tetras.familles = np.array([])

            self.geom.triangles.points = self._path_to_table(self.pointsTR3, 3)
            self.geom.triangles.famille = self.hdf.get_node(self.familleTR3).read()

            self.geom.segments.points = self._path_to_table(self.pointsSE2, 2)
            self.geom.segments.famille = self.hdf.get_node(self.familleSE2).read()

            self.geom.n_segments = len(self.geom.segments.points)
            self.geom.n_triangles = len(self.geom.triangles.points)
            self.geom.n_tetras = len(self.geom.tetras.points)
            print("\tnombre d\'éléments : %s" % self.geom.n_tetras)

            print("\tnombre de segments : %s" % self.geom.n_segments)
            print("\tnombre de triangles : %s" % self.geom.n_triangles)

            if self.med_version_maj == 2:
                if self.med_version_min == 3 and self.med_version_rel == 6:
                    self.extraire_familles_2_3_6()
                elif self.med_version_min == 3 and self.med_version_rel == 4:
                    self.extraire_familles_2_3_4()
            elif self.med_version_maj == 3 or self.med_version_maj == 4:
                self.extraire_familles_2_3_6()
        else:
            print('\t !ce fichier n est pas un med geometrie!')

        self.hdf.close()
        chrono.fin()

        return self.geom

    def extraire_chemins(self):
        if self.med_version_maj == 2:
            noeud_maillage = self.hdf.list_nodes('/ENS_MAA')[0]
            print('\t nom de la geometrie dans .med:', noeud_maillage)
            self.nom_maillage = noeud_maillage._v_pathname.split('/')[-1]

            self.points = '/ENS_MAA/%s/NOE/COO' % self.nom_maillage

            self.pointsTE4 = '/ENS_MAA/%s/MAI/TE4/NOD' % self.nom_maillage
            self.familleTE4 = '/ENS_MAA/%s/MAI/TE4/FAM' % self.nom_maillage

            self.pointsTR3 = '/ENS_MAA/%s/MAI/TR3/NOD' % self.nom_maillage
            self.familleTR3 = '/ENS_MAA/%s/MAI/TR3/FAM' % self.nom_maillage

            self.pointsSE2 = '/ENS_MAA/%s/MAI/SE2/NOD' % self.nom_maillage
            self.familleSE2 = '/ENS_MAA/%s/MAI/SE2/FAM' % self.nom_maillage

            self.liste_famille = '/ENS_MAA/%s/FAS/ELEME' % self.nom_maillage

        elif self.med_version_maj == 3 or self.med_version_maj == 4:
            noeud_maillage = self.hdf.list_nodes('/ENS_MAA')[0]
            self.nom_maillage = noeud_maillage._v_pathname.split('/')[-1]
            noeud_sous_maillage = self.hdf.list_nodes(
                '/ENS_MAA/%s' % self.nom_maillage)[0]

            self.chemin_maillage = noeud_sous_maillage._v_pathname

            self.points = '%s/NOE/COO' % self.chemin_maillage

            self.pointsTE4 = '%s/MAI/TE4/NOD' % self.chemin_maillage
            self.familleTE4 = '%s/MAI/TE4/FAM' % self.chemin_maillage

            self.pointsTR3 = '%s/MAI/TR3/NOD' % self.chemin_maillage
            self.familleTR3 = '%s/MAI/TR3/FAM' % self.chemin_maillage

            self.pointsSE2 = '%s/MAI/SE2/NOD' % self.chemin_maillage
            self.familleSE2 = '%s/MAI/SE2/FAM' % self.chemin_maillage

            self.liste_famille = '/FAS/%s/ELEME' % self.nom_maillage

    def extraire_dic_familles_2_3_4(self):
        lst_nodes = self.hdf.list_nodes(self.liste_famille)
        dic_fam = {}
        lst_nom_familles = []
        for node in lst_nodes:
            if 'GRO' in node._v_children:
                gro = node._v_children['GRO']
                if 'NOM' in gro._v_children:
                    tab_nom = gro._v_children['NOM'].read()
                    nom = tab_to_str(tab_nom)
                    if nom not in lst_nom_familles:
                        lst_nom_familles.append(nom)
                if 'NUM' in node._v_attrs._v_attrnames:
                    fam_num = node._f_getAttr('NUM')
                    dic_fam[fam_num] = nom
        self.lst_nom_familles = lst_nom_familles
        self.lien_num_fam = dic_fam

    def extraire_familles_2_3_4(self):
        self.extraire_dic_familles_2_3_4()
        for i in range(len(self.lst_nom_familles)):
            self.geom.familles[self.lst_nom_familles[i]] = i
        for i in range(self.geom.n_triangles):
            fam_old = self.geom.triangles.famille[i]
            nom_fam = self.lien_num_fam[fam_old]
            self.geom.triangles.famille[i] = self.geom.familles[nom_fam]
        for i in range(self.geom.n_tetras):
            fam_old = self.geom.tetras.famille[i]
            nom_fam = self.lien_num_fam[fam_old]
            self.geom.tetras.famille[i] = self.geom.familles[nom_fam]

    def extraire_familles_2_3_6(self):
        for key in self.hdf.list_nodes(self.liste_famille):
            key = key._v_pathname.split('/')[-1].split('_')
            print('\t\tfamille %s, id %s' % (key[-1], key[1]))
            self.geom.familles[key[-1]] = key[1]

    def extraire_temperature(self):
        if self.med_version_maj == 3:
            chemin = self.hdf.list_nodes('/CHA/TempC')[0]._v_pathname + '/MAI.TE4/MED_NO_PROFILE_INTERNAL'
            # chemin = '/CHA/TempC/00000000000000000018-0000000000000000001/MAI.TE4/MED_NO_PROFILE_INTERNAL'
            # chemin = '/CHA/TempC/00000000000000000101-0000000000000000001/MAI.TE4/MED_NO_PROFILE_INTERNAL'
        else:
            chemin = '/CHA/TempC/MAI.TE4/                   5                  -1/Volume fluide'
        noeud = self.hdf.list_nodes(chemin)[0]
        return noeud.read()

    def extraire_vitesse(self):
        if self.med_version_maj == 3:
            chemin = self.hdf.list_nodes('/CHA/Velocity')[0]._v_pathname + '/MAI.TE4/MED_NO_PROFILE_INTERNAL'
            # chemin = '/CHA/Velocity/00000000000000000018-0000000000000000001/MAI.TE4/MED_NO_PROFILE_INTERNAL'
        else:
            chemin = '/CHA/Vitesse/MAI.TE4/                   5                  -1/Volume fluide'
        noeud = self.hdf.list_nodes(chemin)[0]
        return noeud.read()

    def extraire_donnee(self):
        """
        cree deux dictionnaires contenant les donnees surfaciques
        et volumiques

        """
        lst_champs = self.hdf.list_nodes('CHA')
        nom_champs = []
        for ch in lst_champs:
            nom_champs.append(ch._v_name)
        CHA2D = {}
        for champ in champs:
            try:
                CHA2D[champ] = self.hdf['CHA/' + champ + \
                    '/MAI.TR3/                 100                  -1/Boundary/CO']
                print('importation donnee 2D : %s' % champ)
            except BaseException:
                print('pas de donnee 2d pour le champ %s' % champ)

        CHA3D = {}
        for champ in champs:
            try:
                tt = str(
                    list(self.hdf['/CHA/' + champ + '/MAI.TE4'].keys()))[2:-2]
                chemin = '/CHA/' + champ + '/MAI.TE4/' + tt + '/Fluid volume/CO'
                print(chemin)
            except BaseException:
                print('pas de chemin')
            try:
                CHA3D[champ] = self.hdf[chemin]
                print('importation donnee 3D : %s' % champ)
            except BaseException:
                print('pas de donnee 3d pour le champ %s' % champ)

        self.CHA2D = CHA2D
        self.CHA3D = CHA3D


class CplFile(HdfFile):

    def enregistrer_geom(self, face=True):
        """
        enregistrer la geometrie dans le format perso .cpl

        Ce format permet de ne pas recalculer les connectivites qui ne sont
        pas conservees dans les fichiers .med et .cir
        """
        print('\n\t MODULE hdfFile.py')
        print('\t enregistrement de la geometrie %s' % self.geom.nom)
        print('\t dans le fichier %s' % self.nom)
        print()

        self.face = face

        if self.fichier_existe:
            print('\t -> le fichier existe et sera écrasé')
            os.remove(self.nom)
            self.hdf = tables.open_file(self.nom, 'w')

        self.hdf.create_group('/', 'geom')
        self.hdf.create_group('/geom', 'triangles')

        self.hdf.create_group('/geom', 'tetras')

        # points
        self.hdf.create_array('/geom', 'points',
                             table_to_array(self.geom.points))

        # familles
        self.hdf.create_group('/geom', 'familles')
        for famille in self.geom.familles:
            self.hdf.create_array('/geom/familles', famille,int(self.geom.familles[famille]))

        # triangles
        self.hdf.create_array('/geom/triangles', 'points',
                             table_to_array(self.geom.triangles.points))
        try:
            self.hdf.create_array('/geom/triangles', 'normale',
                                 table_to_array(self.geom.triangles.normale))
        except BaseException:
            print('pas de normales aux triangles (?)')

        try:
            self.hdf.create_array('/geom/triangles', 'cdg',
                                 table_to_array(self.geom.triangles.cdg))
        except BaseException:
            print('pas de centre de gravite aux triangles (?)')

        if len(self.geom.triangles.connectivite_triangles) > 0:
            try:
                self.creer_dataset_regle(
                    '/geom/triangles/connectivite_triangles',
                    self.geom.triangles.connectivite_triangles)
            except BaseException:
                print('\t pas de connectivite triangles')

        if len(self.geom.triangles.connectivite_tetra) > 0:
            try:
                self.hdf.create_array('/geom/triangles', 'connectivite_tetras',
                                     self.geom.triangles.connectivite_tetra)
            except BaseException:
                print('\t pas de connectivite tetras')

        try:
            self.hdf.create_array('/geom/triangles', 'famille',
                                 self.geom.triangles.famille)
        except BaseException:
            print('pas de famille pour les triangles')

        # faces
        self.hdf.create_group('/geom', 'faces')

        if self.face:

            self.creer_dataset_regle('/geom/faces/points',
                                     self.geom.faces.points)
            self.hdf.create_group('/geom/faces', 'liste_trous')

            if len(self.geom.faces.liste_trous) == 0:
                self.geom.faces.liste_trous = [[]] * self.geom.n_faces
            for i in range(len(self.geom.faces.liste_trous)):
                if len(self.geom.faces.liste_trous[i]) != 0:
                    self.creer_dataset_regle(
                        '/geom/faces/liste_trous/t' + str(i),
                        self.geom.faces.liste_trous[i])
                else:
                    self.hdf.create_array(
                        '/geom/faces/liste_trous',
                        't' + str(i),
                        np.array(
                            [0]))
        try:
            self.hdf.create_array('/geom/faces', 'famille',
                                 self.geom.faces.famille)
        except BaseException:
            print('\t -> pas de familles de faces')

        try:
            self.creer_dataset_regle('/geom/faces/liste_triangles',
                                     self.geom.faces.liste_triangles)
        except BaseException:
            print('\t ->pas de liste de triangles')

        # éléments
        if self.geom.n_tetras > 0:
            try:
                self.hdf.create_array('/geom/tetras', 'points',
                                     table_to_array(self.geom.tetras.points))
                self.hdf.create_array('/geom/tetras', 'cdg',
                                     table_to_array(self.geom.tetras.cdg))
                self.hdf.create_array('/geom/tetras', 'famille',
                                     self.geom.tetras.famille)
            except BaseException:
                print('\t pas de donnees volumiques')

        self.hdf.close()

    def charger_geom(self, face=True):
        """
        charge la geometrie stockee dans un format .cpl
        elle peut etre volumique ou surfacique (enveloppe) et peut ou non
        disposer d'informations sur les connectivites
        """

        self.face = face

        print('\n\t MODULE hdfFile.py')
        print('\t loading geometry, stored in the file ')
        print('\t\t -%s -' % self.nom)
        self.geom = Geom(nom='')
        self.geom.points = self._path_to_table('/geom/points', 3)
        self.geom.n_points = len(self.geom.points)

        # familles
        self.geom.familles = {}
        for famille in self.hdf.list_nodes('/geom/familles'):
            nom_famille = famille._v_pathname.split('/')[-1]
            valeur = famille.read()
            self.geom.familles[nom_famille] = valeur
            
            
        print('##############################################################3')
        print(self._path_to_table('/geom/triangles/points', 3))
        
        # triangles
        self.geom.triangles.points = self._path_to_table('/geom/triangles/points', 3)

        try:
            self.geom.triangles.cdg = self._path_to_table('/geom/triangles/cdg', 3)
        except BaseException:
            print('\t -> no center of gravity for triangles')

        try:
            self.geom.triangles.normale = self._path_to_table(
                '/geom/triangles/normale', 3)
        except BaseException:
            print('\t -> pas de normale pour les triangles')

        try:
            self.geom.triangles.connectivite_triangles = \
                self.extraire_dataset_regle('/geom/triangles/connectivite_triangles')
        except BaseException:
            print('\t -> pas de connectivite triangle')

        try:
            self.geom.triangles.connectivite_tetra = \
                self.hdf.get_node('/geom/triangles/connectivite_tetras').read()
        except BaseException:
            print('\t pas de connectivite tetras')

        try:
            self.geom.triangles.famille = \
                self.hdf.get_node('/geom/triangles/famille').read()
        except BaseException:
            print('\t pas des familles pour les triangles')

        self.geom.n_triangles = len(self.geom.triangles.points)

        # faces
        try:
            self.geom.faces.liste_triangles = self.extraire_dataset_regle(
                '/geom/faces/liste_triangles')
        except BaseException:
            print('\t ->pas de liste de triangles')

        self.geom.n_faces = len(self.geom.faces.liste_triangles)

        try:
            self.geom.faces.points = self.extraire_dataset_regle(
                '/geom/faces/points')
            self.geom.n_faces = len(self.geom.faces.points)

            self.geom.faces.liste_trous = [0] * self.geom.n_faces
            for i in range(self.geom.n_faces):
                if type(
                    self.hdf.get_node(
                        '/geom/faces/liste_trous/t' +
                        str(i))) == tables.array.Array:
                    self.geom.faces.liste_trous[int(i)] = []

                else:
                    self.geom.faces.liste_trous[int(i)] = self.extraire_dataset_regle(
                        '/geom/faces/liste_trous/t' + str(i))

            self.geom.faces.famille = self.hdf.get_node(
                '/geom/faces/famille').read()

            self.geom.n_faces = len(self.geom.faces.points)

            no_triangle = []
            for face in range(self.geom.n_faces):
                no_triangle.append(self.geom.faces.liste_triangles[face][0])

            normale = self.geom.triangles.normale[no_triangle]
            self.geom.faces.normale = normale
        except BaseException:
            print('\t ->pas d informations sur les faces')

        try:
            self.geom.faces.famille = self.hdf.get_node(
                '/geom/faces/famille').read()
        except BaseException:
            print('\t -> pas de familles de face')

        # tetras
        try:
            self.geom.tetras.points = self._path_to_table(
                '/geom/tetras/points', 4)
            self.geom.tetras.cdg = self._path_to_table('/geom/tetras/cdg', 3)
            self.geom.tetras.famille = self.hdf.get_node(
                '/geom/tetras/famille').read()
            self.geom.n_tetras = len(self.geom.tetras.points)
        except BaseException:
            print('\t -> pas de donnees volumiques')

        self.hdf.close()

    def creer_dataset_regle(self, nom, liste):
        """
        creer un 'dataset' pour des listes de listes de longueur variable
        avec un array contenant les valeurs et un array contenant les offsets

        pour chaque element de la liste, l'offset defini quelles valeurs font
        partie de cet element
        """
        connect = liste_to_arrays(liste)

        nom_bis = os.path.split(nom)
        self.hdf.create_group(nom_bis[0], nom_bis[1])
        self.hdf.create_array(nom, 'regle', connect[0])
        self.hdf.create_array(nom, 'valeurs', connect[1])

    def extraire_dataset_regle(self, nom):
        """
        recupere une liste de liste de longueur variable stockee dans un dataset
        (dataset : donnee forme d'un array 'valeurs' et d'un array 'offset'
        """
        regle = self.hdf.get_node(nom + '/regle').read()
        valeurs = self.hdf.get_node(nom + '/valeurs').read()

        return arrays_to_liste(regle, valeurs)
