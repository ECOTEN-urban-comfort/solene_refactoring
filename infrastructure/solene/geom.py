#!/usr/bin/python
# -*- coding: utf-8 -*-

# @package geom
# representation interne de la geometrie

"""
Définition d'une classe Geom
contenant la représentation interne de la géométrie

--- Version 0.01 ---

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr

création : 20/01/2010
maj :      04/06/2010

--------------------

contient:
    points = array taille [nb_points, 3]

    triangles = array de taille [nbTriangle, 3]
               + centre de gravité [, 3]
               + surface [, 1]
               + normale [, 3]
              ----
               + lien vers numéro de face
               + lien vers élément contigu
               + lien vers famille

    tetras = array de taille [nb_tetras, 4]
               + centre de gravité [, 3]

              ----
               + lien vers triangle contigu (?)
               + lien vers famille

    familles = dictionnaire {nom_famille : id_famille}

    faces = liste de liste de points [n_points pour chaque face]
               + liste de liste de triangles
               + normale



"""

import numpy as np
import sys
import time

# from hdfFile import *
from infrastructure.solene.utils import Chrono, norm, produit_bool, tab_distance

class Triangles:
    """
    structure contenant les triangles
    """

    def __init__(self):

        self.points = np.array([])
        self.famille = np.array([])
        self.nombre = 0

        self.cdg = np.array([])
        self.normale = []

        self.connectivite_tetra = []
        self.lien_vers_face = []
        self.lien_vers_domain = np.array([])
        self.connectivite_triangles = []
        self.connectivite_triangles_bis = []


class Tetras:
    """
    structure contenant les données des tetraedres
    """

    def __init__(self):

        self.points = np.array([])
        self.famille = np.array([])
        self.lien_vers_domain = np.array([])

        self.cdg = np.array([])


class Segments:
    """
    structures segment
    """

    def __init__(self):
        self.points = np.array([])
        self.famille = np.array([])


class Aretes:
    """
    structure arete
    """

    def __init__(self):
        self.points = np.array([])
        self.lien_aretes_points = []


class Faces:
    """
    structure Faces: contient une liste de point et une liste de
    triangle pour chaque face
    """

    def __init__(self):
        self.points = np.array([])
        self.liste_triangles = []
        self.liste_trous = []
        self.famille = np.array([])
        self.normale = np.array([])

class Geom:
    '''
    test Geom
    '''

    def __init__(self, nom=''):
        self.points = np.array([])
        self.familles = {}
        self.segments = Segments()
        self.triangles = Triangles()
        self.tetras = Tetras()
        self.aretes = Aretes()
        self.faces = Faces()

        self.nom = nom

        self.geom_parent = ''
        self.geom_enfant = []

        self.date_creation = time.time()

        self.n_triangles = 0
        self.n_faces = 0
        self.n_points = 0
        self.n_tetras = 0
        self.n_aretes = 0
        self.n_segments = 0
        self.n_domains = 0

        self.boite = []

        # listes utilisees pour la reconstruiction de la geometrie
        self.liste_segments_points = []
        self.liste_segments = []
        self.liste_sommets = []
        self.liste_points_segments = []
        self.lst_tetras_bords = []

        self.verbose = False

    def sont_alignes(self, segment_i, segment_j):
        """
        renvoi True si les aretes sont alignees
        """
        points1 = self.points[self.segments.points[segment_i][0] - 1] -\
            self.points[self.segments.points[segment_i][1] - 1]

        points2 = self.points[self.segments.points[segment_j][0] - 1] -\
            self.points[self.segments.points[segment_j][1] - 1]
        if abs(np.cross(points1, points2).sum()) > 0.001:
            return False
        else:
            return True

    def distance(self, i, j):
        """
        calcule la distance entre deux points n° i et j
        """
        if isinstance(j, int):
            return norm(self.points[i - 1] - self.points[j - 1])

        elif (type(j)) == np.ndarray:
            return norm(self.points[i - 1] - j)

    def calculer_normale(self):
        """
        calcule les normales de chaque triangle
        """

        normale = []

        for i_point in self.triangles.points:
            A = self.points[i_point[0] - 1]
            B = self.points[i_point[1] - 1]
            C = self.points[i_point[2] - 1]

            AB = B - A
            BC = C - B

            n = np.cross(AB, BC)
            n = n / norm(n)
            n = n.round(4)  # choix du nombre de décimales
            normale.append(n)

        # suppression des -0
        normale = np.array(normale)
        normale[normale == 0] = 0
        self.triangles.normale = normale

    def recuperer_normale_face(self):
        """
        recupere la normale des faces depuis celle des triangles
        """
        premier_triangle = []
        for lst_triangle in self.faces.liste_triangles:
            premier_triangle.append(lst_triangle[0])

        normale = self.triangles.normale[premier_triangle]

        self.faces.normale = normale

    def calculer_cdg(self, triangles=True, tetras=True):
        """
        calcule le centre de gravite de chaque triangle et de chaque
        tetra
        """
        if triangles:
            self.triangles.cdg = np.zeros((self.n_triangles, 3))
            points = self.triangles.points.transpose()
            for i_point in points:
                self.triangles.cdg += self.points[i_point - 1] /\
                    float(len(points))

        if tetras:
            self.tetras.cdg = np.zeros((self.n_tetras, 3))
            points = self.tetras.points.transpose()
            for i_point in points:
                self.tetras.cdg += self.points[i_point - 1] /\
                    float(len(points))

    def verifier_normale(self):
        """
        verifie l'orientation des normale d'apres le type
        du tetraedre adjacent
        """
        for i in range(self.n_triangles):
            AB = self.tetras.cdg[int(self.triangles.connectivite_tetra[i])] - self.triangles.cdg[i]

            # inverse le sens de la normale si elle n'est pas orientée vers
            # l'élément contigu
            if np.inner(AB, self.triangles.normale[i]) < 0.:
                self.triangles.normale[i] = -self.triangles.normale[i]

    def extraire_boite(self, tetras_ou_triangles='tetras'):
        """
        condition sur les points
        renvoi la liste des points réduite en ne prenant en compte
        que les points dans la boite ainsi que les liens de chaque
        élément vers la liste complète
        """
        if tetras_ou_triangles == 'tetras':
            points = self.tetras.points
        elif tetras_ou_triangles == 'triangles':
            points = self.triangles.points

        # extrait le premier point de chaque triangle/tetra et crée une table
        # avec leurs coordonnées
        points1 = points.transpose()[0]
        points1 = self.points[points1 - 1]

        # crée deux tableau de booléens
        tt = (points1 >= self.boite[0]).transpose()
        ttt = (points1 <= self.boite[1]).transpose()
        condition = tt[0] * tt[1] * tt[2] * ttt[0] * ttt[1] * ttt[2]

        liste_bis = points.compress(condition, 0)
        lien = np.arange(self.n_tetras).compress(condition)

        return liste_bis, lien


# ==============================================================================
# CONNECTIVITE

    def connecter_triangle_triangle(self, face=True):
        """
        pour chaque triangle, donne la liste des triangles adjacents
        qui appartiennent à la même face
        """
        chrono = Chrono('connecter_triangle_triangle', 'geom')
        tableau_connectivite_triangles = []
        for i in range(self.n_triangles):

            triangles = self.triangles.points[i]

            triangles_liees = []

            liste_aretes = [[triangles[0], triangles[1]],
                            [triangles[1], triangles[2]],
                            [triangles[2], triangles[0]]]

            face = np.copy(self.triangles.points)
            face[i] = [0, 0, 0]
            for arete in liste_aretes:
                tt = ((arete[0] == face) +
                      (arete[1] == face)).transpose()
                triangles_liees.append(np.argmax(np.int32(tt[0]) +
                                                 np.int32(tt[1]) +
                                                 np.int32(tt[2])))
            tableau_connectivite_triangles.append(triangles_liees)

        self.triangles.connectivite_triangles = tableau_connectivite_triangles

        self._connecter_triangle_triangle_arete()
        chrono.fin()

    def _connecter_triangle_triangle_arete(self):
        """
        deconnecte les triangles qui sont relie par deux points appartenant
        a une arete : permet de ne laisser que les triangles connecte sur
        la meme face

        utilise par self.connecter_triangle_triangle()
        """
        # connectivite triangles bis : les triangles ne sont pas connectes par
        # deux points qui forment une aretes (cad ils appartiennent a la meme
        # face
        chrono = Chrono('connecter_triangle_triangle', 'geom')
        tab_conn_bis = []
        segm = self.segments.points

        segm_bis = []
        for segm_i in segm:
            segm_bis.append(set(segm_i))

        tableau_connectivite_triangles = self.triangles.connectivite_triangles
        normale = self.triangles.normale

        for i in range(self.n_triangles):

            points_tri = set(self.triangles.points[i])
            ligne_conn_bis = []
            for triangle_lie in tableau_connectivite_triangles[i]:
                points_tri_lie = set(self.triangles.points[triangle_lie])
                ttt = points_tri.intersection(points_tri_lie)
                if ttt not in segm_bis:
                    if (normale[i] == normale[triangle_lie]).all():
                        ligne_conn_bis.append(triangle_lie)
            tab_conn_bis.append(ligne_conn_bis)

        self.triangles.connectivite_triangles_bis = tab_conn_bis
        chrono.fin()

    def _trouver_tetra_bord(self):
        """
        trouve les tetraedres qui sont contigus a une face de bord
        utilise pour limiter la liste des tetraedres a connecter aux triangles

        utilise par : self.connecter_triangle_tetra()
        """
        tetras = self.tetras.points
        lst_pts_bords = self.triangles.points.flatten()
        set_pts_bords = set(lst_pts_bords)

        lst_tetras_bords = []
        for i_tetra, tetra in enumerate(tetras):
            set_pts_tetra = set(tetra)
            if len(set_pts_tetra.intersection(set_pts_bords)) > 2:
                lst_tetras_bords.append(i_tetra)

        self.lst_tetras_bords = np.array(lst_tetras_bords)

    def connecter_triangle_tetra(self,
                                 famille_face=None,
                                 famille_elem=None,
                                 boite=False,
                                 debug=False):
        """
        cree le vecteur connectivite_tetra qui donne pour chaque triangles
        le lien vers l'élément contigu,
        ne prends en compte que les triangles de -familleFace-
        et les tetraedres de -famille_elem-
        """

        self._trouver_tetra_bord()

        chrono = Chrono('connecter_triangle_tetra', 'geom')

        # crée des listes réduites de points comprise dans la boîte
        # en conservant les liens

        if boite:
            face, lien_face = self.extraire_boite('triangles')
            elem, lien_elem = self.extraire_boite('tetras')

        else:
            face = self.triangles.points
            elem = self.tetras.points
            lien_face = np.arange(self.n_triangles)
            lien_elem = np.arange(self.n_tetras)

        if len(self.lst_tetras_bords) > 0:
            lst_tetras = self.lst_tetras_bords
            elem = self.tetras.points[lst_tetras]
            lien_elem = lst_tetras

        # réduit encore aux éléments de la famille 'famille'
        if famille_elem:
            fam = int(self.familles[famille_elem])
            elem = self.tetras.points[self.tetras.famille == fam]
            lien_elem = np.arange(self.n_tetras)[self.tetras.famille == fam]

        if famille_face:
            ok = np.zeros(self.n_triangles)
            for fami in famille_face:
                fam = int(self.familles[fami])
                ok = ok + np.int32(self.triangles.famille[lien_face] == fam)

            ok = np.bool_(ok)

            face = self.triangles.points[ok]
            lien_face = lien_face[ok]

        if debug:
            vec_debug = np.zeros(self.n_triangles)

        self.triangles.connectivite_tetra = np.zeros(self.n_triangles)

        len_face = len(face)
        for i in range(len_face):

            bu = self._trouver_tetra_connecte(face, elem, lien_elem, i)
            if len(bu) == 0:
                if debug:
                    vec_debug[i] = 1
            try:
                self.triangles.connectivite_tetra[lien_face[i]] = int(bu[0])
            except BaseException:
                self.triangles.connectivite_tetra[lien_face[i]] = None
        chrono.fin()
        if debug:
            return vec_debug

    def _trouver_tetra_connecte(self, face, elem, lien_elem, i):
        """
        trouve la liste des tetraedes connectés au triangle (len=1 ou 2)

        utilise par : self.connecter_triangle_tetra()
        """
        # opération spéciale...
        # créer un tableau de booléen pour chaque triangle :
        #     (face[i][i]==elem) renvoi un tableau de la dimension
        #     de elem avec True si le point j de la face i est dans elem
        # puis transpose pour pouvoir additionner
        tt = (np.int32(face[i][0] == elem) +
              np.int32(face[i][1] == elem) +
              np.int32(face[i][2] == elem))

        tt = tt.transpose()

        # fait la somme des lignes du tableau: cherche l'élément pour
        # lequel la somme est maximale (=3, signifie que les trois points
        # de la face sont dans les 4 points de l'éléments : solution unique)
        # renvoie le lien vers la liste complète des éléments

        ga = (tt[0] +
              tt[1] +
              tt[2] +
              tt[3])

        bu = lien_elem[ga == 3]

        # TODO
        if len(bu) > 1:
            try:
                bu = bu[self.tetras.famille[bu] != int(self.familles['arbre'])]
            except BaseException:
                pass

        return bu

#
# Fin connectivite
# ==============================================================================

# ==============================================================================
# recuperation des segments

# ==============================================================================

# ==============================================================================
# = DEBUT reconstruction des arêtes
#

    def reconstruire_aretes(self):
        """
        reconstruit la liste des arêtes

        méthode :
            ** trouve les points sommets
            ** pour chaque sommet, parcours chaque arete jusqu'au
            prochain sommet puis les ajoute à la liste
        remarque :
            fonctionne tant que la modélisation est volumique et qu'il
            n'existe pas de "mur" sans épaisseur
        """

        self._trouver_sommets()
        liste_sommets = self.liste_sommets

        liste_segments = self.liste_segments

        points_aretes_bis = []
        points_bis = []

        for point in liste_sommets:
            for segment in liste_segments[point]:
                extremite = self._trouver_extremite_arete(point, segment)
                points_aretes_bis.append(extremite[2])
                points_bis.append([point, extremite[0]])

        points_aretes = []
        points = []

        for i, arete in enumerate(points_bis):
            arete.sort()
            if arete not in points:
                points.append(arete)
                points_aretes.append(points_aretes_bis[i])

        self.n_aretes = len(points)
        self.aretes.lien_aretes_points = points_aretes
        self.aretes.points = np.array(points)

    def _trouver_sommets(self):
        """
        trouve la liste des sommets, c'est-à-dire la liste des points qui
        appartiennent à plus de deux segments, ou dont les deux segments ne
        sont pas alignés

        utilise par : self.recontruire_aretes()
        """

        liste_points_segments_tous = self.segments.points.flatten().tolist()
        liste_points_segments = list(set(liste_points_segments_tous))
        self.liste_points_segments = liste_points_segments

        liste_segments = {}
        for point in liste_points_segments:
            liste_segments[point] = self._trouver_segments_point(
                point).tolist()

        self.liste_segments = liste_segments

        liste_sommets = []
        for i in list(liste_segments.keys()):
            if len(liste_segments[i]) > 2:
                liste_sommets.append(i)
            else:
                segment_1 = liste_segments[i][0]
                segment_2 = liste_segments[i][1]
                if not self.sont_alignes(segment_1, segment_2):
                    liste_sommets.append(i)
        self.liste_sommets = liste_sommets

    def _trouver_extremite_arete(self, point, segment):
        """
        trouver le point extrémité de l'arête dont l'autre extrémité est -point-
        dans la direction de -segment-

        utilise par : self.recontruire_aretes()
        """
        point_suivant = self._trouver_point_suivant(point, segment)
        liste = [point, point_suivant]
        while point_suivant not in self.liste_sommets:
            segment = self._trouver_segment_suivant(point_suivant, segment)
            point_suivant = self._trouver_point_suivant(point_suivant, segment)
            liste.append(point_suivant)
        return point_suivant, segment, liste

    def _trouver_point_suivant(self, point, segment):
        """
        trouver le point suivant du côté de segment

        utilise par : self._trouver_extremite_arete
        """
        tt = self.segments.points[segment].tolist()
        tt.remove(point)
        return tt[0]

    def _trouver_segment_suivant(self, point, segment):
        """
        trouve le segment entre -segment- et -point suivant-

        utilise par : self._trouver_extremite_arete
        """
        tt = self._trouver_segments_point(point).tolist()
        tt.remove(segment)
        return tt[0]

    def _trouver_segments_point(self, point):
        """
        trouver les segments dont le point est une extrémité:
        2 si c'est un point courant
        plus de 2 si c'est un sommet

        utilise par : self._trouver_segment_suivant()
        """
        points = self.segments.points.transpose()
        positions_segments = ((point == points[0]) + (point == points[1])) == 1
        return np.arange(self.n_segments)[positions_segments]

# = FIN recontruction des aretes
# ==============================================================================


# =============================================================================
# Reconstruction des faces
#

    def reconstruire_faces(self):
        """
        nécessaire : tableau de connectivite Triangles-Triangles
            tableau de connectivite Triangles-Triangles(Bis)
            liste des sommets
            reconstruction des aretes

        méthode : pour chaque triangle pas encore attribuée
            -> trouver toutes les triangles reliées par deux points qui
                ne sont pas sur une arete
            -> recuperer les sommets parmis les points de ces triangles
            -> reconstruire les contours parmis ces sommets
            -> séparer les contours extérieurs et les contours trous
            -> attribuer les contours et les triangles à une nouvelle
                face
        """

        chrono = Chrono('reconstruire_faces', 'geom')
        # initialisation
        self.faces.points = []
        self.faces.liste_trous = []
        self.faces.liste_triangles = []

        triangles = self.triangles.points

        stop = False
        triangles_restants = list(range(self.n_triangles))

        while stop == False:

            # choisir la première triangles parmis celles qui ne sont
            # pas encore attribuées
            triangle = triangles_restants[0]

            # trouver les triangles reliées par deux points qui ne sont pas
            # sur une arete
            triangles_relies = self._trouver_triangles_relies(triangle)

            # supprimer les faces qui vont être attribué de la liste
            # des faces restantes
            triangles_restants = list(set(triangles_restants)
                                      .difference(set(triangles_relies)))

            if len(triangles_restants) == 0:
                # finito
                stop = True

            if len(triangles_relies) == 0:
                parcours_ext = self.triangles.points[triangle]
                triangles_relies = [triangle]
                trous = []
                triangles_restants.remove(triangle)
            else:
                # récupérer les points des triangles de la face
                points_face = list(set(triangles[triangles_relies]
                                       .flatten().tolist()))

                # parmis les points de la face, récuperons les sommets
                liste_sommets_face0 = []
                for p in points_face:
                    if p in self.liste_sommets:
                        liste_sommets_face0.append(p)

                # on a la liste des sommets, on peut reconstruire les contours
                liste_parcours = self._trouver_contours(liste_sommets_face0,
                                                        points_face)

                # parmis ces contours, seul le plus grand n'est pas un trou
                if len(liste_parcours) > 1:
                    parcours_ext, trous = self._trouver_parcours_exterieur(liste_parcours)

                else:
                    parcours_ext = liste_parcours[0]
                    trous = []

            # attribuons ces résultats à une nouvelle face
            self.faces.points.append(parcours_ext)
            self.faces.liste_trous.append(trous)
            self.faces.liste_triangles.append(triangles_relies)

        self.n_faces = len(self.faces.points)
        self.trouver_familles()
        self.recuperer_normale_face()

        chrono.fin()

    def _trouver_triangles_relies(self, triangle):
        """
        trouve les triangles reliés à -triangle- par deux points qui ne sont
        pas sur une arete

        utilise self.connectivite_triangle_bis

        utilisé par : self.recontruire_faces()
        """
        try:
            con_triangles = self.triangles.connectivite_triangles_bis
        except AttributeError:
            self.connecter_triangle_triangle()
            con_triangles = self.triangles.connectivite_triangles_bis

        liste_triangles_reliees = []

        nvelles_triangles = con_triangles[triangle]

        # tant que la boucle amène de nouvelles triangles
        while len(nvelles_triangles) != 0:
            nvelles_triangles_bis = np.copy(nvelles_triangles)
            nvelles_triangles = []

            # si les nouvelles triangles ne sont pas déjà ajouté,
            # on les ajoute a la liste
            for triangles in nvelles_triangles_bis:
                for j in con_triangles[triangles]:
                    if j not in liste_triangles_reliees:
                        liste_triangles_reliees.append(j)
                        nvelles_triangles.append(j)

        return liste_triangles_reliees

    def _trouver_contours(self, liste_sommets, points):
        """
        retourne la liste des parcours dans -liste_sommets-

        on a besoin de tous les points de la face -points- pour
        éviter les aretes qui n'appartiennent pas à la face

        utilisé par : self.reconstruire_faces()
        """
        parcours = []

        while len(liste_sommets) > 1:
            nv_parcours, liste_sommets = self._trouver_contour(liste_sommets,
                                                               points)
            parcours.append(nv_parcours)

        return parcours

    def _trouver_contour(self, liste_sommets, points):
        """
        retourne un nouveau parcours contenu dans -liste_sommets- ainsi que
        la liste des sommets restants

        on a besoin de tous les points de la face -points- pour
        éviter les aretes qui n'appartiennent pas à la face

        utilisé par : self._trouver_contours
        """
        stop = False

        # initisalisation : gardons en memoire le premier sommet
        sommet = liste_sommets[0]
        sommet0 = sommet
        enchainement_parcours = [sommet]
        liste_sommets.remove(sommet)
        iteration = 0

        while stop == False and len(liste_sommets) != 0:
            iteration += 1
            for i in liste_sommets:
                ga = self._est_une_arete(i, sommet)
                if isinstance(ga, np.ndarray) and ga.size == 1:
                    ga = ga.item()
                bu = set(self.aretes.lien_aretes_points[ga]).issubset(set(points))

                # si tt est une arete et que celle ci est dans la face
                # (i.e. ses points sont dans la liste des points)
                if isinstance(ga, int) and bu:
                    if i == sommet0:
                        stop = True
                    else:

                        enchainement_parcours.append(i)
                        liste_sommets.remove(i)
                        sommet = i

                if len(liste_sommets) == 0:
                    stop = True
                    break

            if len(enchainement_parcours) > 2 and\
                    self._est_une_arete(sommet, sommet0):
                stop = True

            # old debug (?)
            if iteration > 200:
                stop = True

        return enchainement_parcours, liste_sommets

    def _est_une_arete(self, i, j):
        """
        retourne le numéro de l'arete si -i- et -j- forment une arete,
        False sinon

        i et j sont deux numéros de points

        utilisé par : self._trouver_contour()
        """
        a = [i, j]

        tt = produit_bool(a == self.aretes.points)
        a.sort()
        ttt = produit_bool(a == self.aretes.points)
        if ttt.sum() == 1:
            return np.arange(self.n_aretes)[ttt == 1]
        elif tt.sum() == 1:
            return np.arange(self.n_aretes)[tt == 1]
        else:
            return 0

    def _trouver_parcours_exterieur(self, liste_parcours):
        """
        renvois le parcours qui a les points les plus distants, puis les autres

        utilisé par : self.recontruire_faces()
        """
        distance_max = []
        for i in liste_parcours:
            distance_max.append(self._trouver_distance_max(i))

        parcours_ext = liste_parcours[distance_max.index(max(distance_max))]
        liste_parcours.remove(parcours_ext)

        return parcours_ext, liste_parcours

    def _trouver_distance_max(self, contour):
        """
        renvoie la plus grande distance entre tous les points du contours

        utilisé par self._trouver_parcours_exterieur()
        """
        distances = []
        for i in contour:
            for j in contour:
                if i != j:
                    distances.append(self.distance(i, j))

        return max(distances)

    def trouver_familles(self):
        """
        retrouve la famille de chaque face en fonction de celle de ces triangles

        utilisé par self.reconstruire_faces()
        """
        self.faces.famille = np.zeros(self.n_faces)
        for face in range(self.n_faces):
            ga = self.faces.liste_triangles[face]
            famille = self.triangles.famille[ga[0]]
            if (self.triangles.famille[ga] ==
                    famille * np.ones(len(ga))).all():
                self.faces.famille[face] = famille


# Fin reconstruction des faces
# =============================================================================

# =============================================================================
# SELECTION  EXTRACTION GEOM

    def selection_famille(self,
                          liste_famille,
                          faces=True):
        """
        renvoi une liste de booleen qui dit si les faces ou les
        triangles appartiennent à la liste des familles
        """

        if isinstance(liste_famille, int):
            liste_famille = [liste_famille]
        elif isinstance(liste_famille, str):
            if liste_famille in list(self.familles.keys()):
                liste_famille = int(self.familles[str])
            else:
                liste_famille = []
        elif isinstance(liste_famille[0], type(liste_famille[0])) == str:
            liste_famille_bis = []
            for i in liste_famille:
                if i in list(self.familles.keys()):
                    liste_famille_bis.append(int(self.familles[i]))
            liste_famille = liste_famille_bis

        ok_fc = np.zeros(self.n_faces)
        ok_ft = np.zeros(self.n_triangles)

        for i_fam in liste_famille:
            ok_fc = ok_fc + np.int32(np.int32(self.faces.famille) == i_fam)
            ok_ft = ok_ft + np.int32(self.triangles.famille == i_fam)

        ok_fc = np.bool_(ok_fc)
        ok_ft = np.bool_(ok_ft)

        if faces:
            return ok_fc, ok_ft
        else:
            return ok_ft

# ==============================================================================
# Recontruction totale

    def reconstruire_geom(self):
        """
        reconstruction de la geometrie importee en med
        """
        chrono = Chrono('MACRO : reconstruction_geom', 'geom')

        self.calculer_cdg()
        self.calculer_normale()
        self.connecter_triangle_tetra()
        self.connecter_triangle_triangle()
        self.verifier_normale()

        self.reconstruire_aretes()
        self.reconstruire_faces()

        chrono.fin()

# ==============================================================================
# Gestion des 'sous-géometries'

    def creer_sous_geom(self, nom='', liste_famille=None):
        """
        cree une geometrie surfacique enfant en selectionnant par
        famille
        """
        if liste_famille:
            sous_geom = self.selection_famille(liste_famille)

        geom = self.exporter_sous_geom(sous_geom)

        geom.nom = nom
        geom.geom_parent = self.nom
        self.geom_enfant.append(geom.nom)

        return geom

    def exporter_sous_geom(self, sous_geom, volum=False):
        """
        cree une nouvelle geometrie extraite de self en fonction d'un tableau
        booleen : faces|triangles ( + |tetras s'il y a lieu)

        exporte points, triangles, faces,
        """

        bool_face = sous_geom[0]
        bool_triangle = sous_geom[1]

        liste_triangle = np.arange(self.n_triangles)[bool_triangle]
#        liste_face = np.arange(self.n_faces)[bool_face]

        geom = Geom()
        geom.points = self.points
        geom.n_points = self.n_points

        geom.familles = self.familles

        geom.triangles.points = self.triangles.points[bool_triangle]
        geom.triangles.cdg = self.triangles.cdg[bool_triangle]
        geom.triangles.normale = self.triangles.normale[bool_triangle]
        geom.triangles.famille = self.triangles.famille[bool_triangle]

        # la connectivite tetra correspond aux index de la geometrie mère
        geom.triangles.connectivite_tetra = \
            self.triangles.connectivite_tetra[bool_triangle]

        # la connectivite triangle correspond aux index de la geometrie
        # fille (si la connectivite triangle mere a été conservée)

        try:
            geom.triangles.connectivite_triangles = []
            for i in range(self.n_triangles):
                if bool_triangle[i]:
                    ligne = []
                    for j in self.triangles.connectivite_triangles[i]:
                        ligne.append(liste_triangle.searchsorted(
                            self.triangles.connectivite_triangles[i][j]))
                    geom.triangles.connectivite_triangles.append(ligne)
        except BaseException:
            pass

        geom.faces.points = []
        geom.faces.liste_trous = []
        geom.faces.liste_triangles = []
        geom.faces.famille = []

        face_bis = []
        for i in range(self.n_faces):
            if bool_face[i]:
                ligne = []
                for j in self.faces.liste_triangles[i]:
                    ligne.append(liste_triangle.searchsorted(j))
                face_bis.append(ligne)
            else:
                face_bis.append([0])

        for i in range(self.n_faces):
            if bool_face[i]:
                geom.faces.points.append(self.faces.points[i])
                geom.faces.liste_trous.append(self.faces.liste_trous[i])
                geom.faces.liste_triangles.append(face_bis[i])
                geom.faces.famille.append(self.faces.famille[i])

        geom.n_triangles = len(geom.triangles.points)
        geom.n_faces = len(geom.faces.points)

        geom.recuperer_normale_face()

        if volum:
            bool_elem = sous_geom[2]
            geom.tetras.points = self.tetras.points[bool_elem]
            geom.tetras.cdg = self.tetras.cdg[bool_elem]
            geom.tetras.famille = self.tetras.famille[bool_elem]
            geom.n_tetras = len(self.tetras.points)

        return geom