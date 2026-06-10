#!/usr/bin/python
# -*- coding: utf-8 -*-

"""@package solFile
===============================================================================
=  =                             solFile.py                                 = =
===============================================================================

Définition d'une classe SolFile de lecture et d'ecriture
des fichiers Solene

Version 0.01

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr

création : 14/01/2010
maj :      04/06/2010

clean :    27/07/2010
"""
import sys

import numpy as np
from infrastructure.solene.utils import ecrire_fichier, trouver_lien, Chrono
from infrastructure.solene.geom import Geom


def ecrire_ligne(arr):
    """
    convertir un 'array' en chaine (sans les crochets)
    """

    chaine = np.array2string(arr)
    chaine = chaine[1:len(chaine) - 1]
    chaine = chaine.split()
    ligne = ''
    for i in range(len(chaine)):
        ligne += str(float(chaine[i])) + ' '

    return ligne


def read_val(name, geom=None):
    """
    return the values from the val file -name-
    with the shape of the geometry -geom-

    args :
        name:string
        (geom:geom.Geom)

    return :
        numpy.array
    """
    val = ValFile(name, geom=geom)
    val.lire()
    sortie = val.extraire_val()
    val = None
    return sortie


def write_val(name, geom, data, faces=False):
    """
    export the data with the shape of the geometry -geom-
    to the val file -val-
    if -data- is a single value, this same value is assigned to each patch

    args :
        name:string
        geom:geom.Geom
        data:float|list|numpy.array
        (faces:bool)
    """
    if name[-4:] != '.val':
        name += '.val'

    val = ValFile(name, geom=geom)

    if faces:
        n_elem = geom.n_faces
        ecrire_descripteur = val.ecrire_descripteur_face
    else:
        n_elem = geom.n_triangles
        ecrire_descripteur = val.ecrire_descripteur_cont

    # if data is a float
    try:
        valeurs = [float(data)] * n_elem
    except BaseException:
        valeurs = data

    ecrire_descripteur(valeurs)
    val = None


def read_cir(name, mask=None, faces=False):
    """
    return the geometry in the cir file -name-
    eventualy, use the non-triangulated geometry
    in the cir file -mask-

    args :
        name:string
        (mask:string)

    return :
        geom.Geom
    """
    cir = CirFile(name)
    cir.lire()

    if faces:
        cir.extraire_geom_face()
    else:
        cir.extraire_geom_triangle()

    if mask:
        cir2 = CirFile(mask)
        cir2.lire()
        cir2.extraire_geom_face()

        points_tr = cir.geom.points
        points_fa = cir2.geom.points
        table = []
        for face in cir2.geom.faces.points:
            ligne = []
            for point in face:
                for i in range(len(points_tr)):
                    if (points_fa[point - 1] == points_tr[i]).all():
                        ligne.append(i + 1)
            table.append(ligne)

        cir.geom.faces.points = table

    return cir.geom


def write_cir(name, geom, faces=False):
    """
    export the geometry -geom- in the cir file -name-
    use meshed geometry (face = False, defaut)
    or non-meshed geometry (face = True)

    args :
        name:string
        geom:geom.Geom
        (faces:bool)
    """
    if name[-4:] != '.cir':
        name += '.cir'

    cir = CirFile(name)
    cir.geom = geom
    cir.exporter(faces=faces)


class InterpolVal:
    """
    classe d'interpolation pour passer
    les donnees entre deux geometries/deux maillages
    """

    def __init__(self, geom_cible, geom_source):
        self.geom_cible = geom_cible
        self.geom_source = geom_source
        self.lien = trouver_lien(geom_cible, geom_source).tolist()

    def interpoler(self, fichier_source, fichier_cible):
        val_in = read_val(fichier_source, self.geom_source)
        val_out = val_in[self.lien]
        write_val(fichier_cible, self.geom_cible, val_out)


class ValFile:
    """classe de document solene valeur"""

    def __init__(self, fname, geom=None):
        if fname[-4:] != '.val':
            fname += '.val'

        self.fname = fname
        self.val = []

        self.triangule = True

        if geom:
            self.geom = geom
            self.n_faces = geom.n_faces
        else:
            self.n_faces = 0

        self.val_min = 0
        self.val_max = 0

    def lire(self):
        """
        dechiffre le fichier val,
        stocke dans un dictionnaire
        sans la transcription dans la representation interne
        """

        sol_file = open(self.fname, 'r')
        entete = sol_file.readline().split()
        liste_n_triangles = []
        if len(entete) == 4:
            self.n_faces = int(entete[0])
            self.val_min = float(entete[2])
            self.val_max = float(entete[3])
            face = 0
            while face < self.n_faces:
                ligne_vec = sol_file.readline().split()
                if ligne_vec[0][0] == 'f':
                    n_triangles = int(ligne_vec[1])
                    liste_n_triangles.append(n_triangles)
                    valeur_face = []
                    for triangle in range(n_triangles):
                        valeur_face.append(float(sol_file.readline()))
                self.val.append(valeur_face)
                face += 1

        if max(liste_n_triangles) == 1:
            self.triangule = False
        sol_file.close()

    def extraire_val(self):
        """
        extrait les valeurs
        """
        if not self.geom:
            return self.extraire_val_sans_geom()
        elif self.triangule:
            return self.extraire_val_cont()
        else:
            return self.extraire_val_face()

    def extraire_val_sans_geom(self):
        """
        extrait les valeurs d'un val dans le meme ordre qu'elles sont stockees
        """
        export_val = []
        for faces in range(self.val):
            for contour in range(faces):
                export_val.append(contour)

        return export_val

    def extraire_val_cont(self):
        """
        extrait les valeurs par contour
        """
        lien = self.geom.faces.liste_triangles

        for j in range(len(lien)):
            if len(lien[j]) != len(self.val[j]):
                print('la geométrie ne correspond pas à la table de valeur')

        export_val = np.zeros(self.geom.n_triangles)
        ok = np.zeros(self.geom.n_triangles)

        # retrouve l'ordre des triangles correspondant à la représentation
        # interne
        for j in range(len(lien)):
            for i in range(len(lien[j])):
                export_val[lien[j][i]] = self.val[j][i]
                ok[lien[j][i]] = 1

        if min(ok) == 0:
            print("la geometrie n'est pas complete")
            print(len(ok), ok.sum())
            for i in range(len(ok)):
                if ok[i] == 0:
                    export_val[i] = 10000

        return export_val

    def extraire_val_face(self):
        """
        extrait les valeurs par face
        """
        lien = self.geom.faces.liste_triangles

        export_val = np.zeros(self.geom.n_triangles)

        for j in range(len(lien)):
            for i in range(len(lien[j])):
                export_val[lien[j][i]] = self.val[j][0]

        return export_val

    def ecrire_descripteur_cont(self,
                                valeurs,
                                bool_face=None):
        """
        exporte un -val- contenant les valeurs contenues dans
            valeurs:array(float) (liste des valeurs par triangle
            dans l'ordre de -self.geom-)
        """

        if bool_face is None:
            n_faces = self.geom.n_faces
            face_max = n_faces
            liste_face = np.arange(n_faces)
        else:
            liste_face = np.arange(self.geom.n_faces)[bool_face]
            n_faces = len(liste_face)
            face_max = max(liste_face) + 1

        descripteur = '%s %s \t %s %s\n' % (n_faces,
                                            face_max,
                                            min(valeurs),
                                            max(valeurs))

        # test coherence geometrie/valeurs
        if len(valeurs) != self.geom.n_triangles:
            print('la geometrie et la liste de valeurs ne correspondent pas:')
            print('\t len(valeurs): ', len(valeurs))
            print('\t nombre de triangles: ', self.geom.n_triangles)

        for i in range(n_faces):
            n_contours = len(self.geom.faces.liste_triangles[liste_face[i]])
            descripteur += 'f%s %s\n' % (i + 1, n_contours)
            for j in range(n_contours):
                contours = self.geom.faces.liste_triangles[i][j]
                descripteur += '\t %s\n' % valeurs[contours]

        ecrire_fichier(self.fname, descripteur)

    def ecrire_descripteur_face(self, valeurs):
        """
        exporte un -val- contenant les valeurs contenues dans valeurs
        non triangulees
        """

        # test coherence geometrie/valeurs
        if len(valeurs) != self.geom.n_faces:
            print('la geometrie et la liste de valeurs ne correspondent pas:')
            print('\t len(valeurs): ', len(valeurs))
            print('\t nombre de faces: ', self.geom.n_faces)

        descripteur = '%s %s \t %s %s\n' % (self.n_faces,
                                            self.n_faces,
                                            min(valeurs),
                                            max(valeurs))

        for face in range(self.geom.n_faces):
            descripteur += 'f%s 1\n%s\n' % (face + 1, valeurs[face])

        ecrire_fichier(self.fname, descripteur)

# MACROS
    def creer_descripteur_constant(self, valeur):
        """
        cree un descripteur avec une valeur unique pour tous les triangles
        """
        valeurs = np.ones(self.geom.n_triangles) * valeur

        self.ecrire_descripteur_cont(valeurs)

    def creer_descripteur_par_face(self, listes_faces, liste_valeurs):
        """
        cree un descripteur avec une valeur pour chaque ensemble de face
        de listes_faces (repere par leur numero)

        exemple

        listes_faces  = [[1,2,45], [3,4], [6], [7,10]]
        liste_valeurs = [  0.2    , 0.5 , 0.3,  0.12 ]
        """
        valeurs = np.zeros(self.geom.n_triangles)
        for i in range(len(listes_faces)):
            for face in listes_faces[i]:
                valeurs[self.geom.faces.liste_triangles[face]] = liste_valeurs[i]

        self.ecrire_descripteur_cont(valeurs)


class CirFile:
    """
    classe de document solene geometrie (.cir)

    ** pour extraire une geometrie d'un -cir- triangule existant:
        cir=CirFile(nomDuFichier)
        cir.lire()
        cir.extraire_geom_triangle()

        la geometrie est alors cir.geom
        limite : ne recupere pas les points extremites des faces

    ** pour exporter un -cir- d'une geometrie -geom- existante :
        cir=CirFile(nomDuFichier)
        cir.importer_geom(geom)
        cir.exporter(face=False)      -> geometrie triangulee
         (ou cir.exporter(face=_true)  -> geometrie non triangulee

    """

    def __init__(self, fname):
        self.fname = fname
        self.cir = []
        self.n_faces = 0

        self.geom = Geom()

    def lire(self):
        """
        lecture du fichier .cir

        stocke dans une representation intermediaire
        manque la transcription dans le format de représentation interne
        
        Reading the .cir file

        Stored in an intermediate representation
        Missing transcription in the internal representation format
        """
        sol_file = open(self.fname, 'r')
        liste_sol_file = sol_file.readlines()
        table_cir = []
        for i in range(len(liste_sol_file)):

            # vérification lignes vides
            if not liste_sol_file[i].isspace():
                table_cir.append(liste_sol_file[i].split())

        self.n_faces = int(table_cir[0][0])
        ligne = 6
        face = 0

        # chaque face est un dictionnaire
        # contenant une entree 'contours' qui contient une liste de matrices
        # représentant les coordonnées des points de chaque contour
        while face < self.n_faces:
            dic_face = {}
            # on repere le début de la face
            if table_cir[ligne][0][0] == 'f':
                # recupere les informations la face
                dic_face['numeroFace'] = int(table_cir[ligne][0][1:])
                dic_face['nContours'] = int(table_cir[ligne][1])
                dic_face['normale'] = [float(table_cir[ligne + 1][j])
                                       for j in range(3)]
                dic_face['contours'] = []
                dic_face['nTrous'] = 0
                dic_face['trous'] = []
                ligne += 2

                # recuperation des coordonnées des points de chaque contour
                # sous forme de matrice 3x3
                for cont in range(dic_face['nContours']):
                    mat_points = []
                    if 'c' in table_cir[ligne][0]:
                        n_trous = int(table_cir[ligne][0][1:])
                        dic_face['nTrous'] = n_trous
                        n_points = int(table_cir[ligne + 1][0]) - 1
                        # -1 car pas besoin du point en plus pour fermer la boucle
                        for i in range(n_points):
                            point = []
                            for j in range(3):
                                point.append(
                                    float(table_cir[ligne + 2 + i][j]))
                            mat_points.append(point)
                    dic_face['contours'].append(mat_points)
                    ligne += n_points + 3
                    # +3 (ligne c, ligne nombre de point, ligne point en plus pour fermer la boucle
                    for t in range(n_trous):
                        mat_points = []
                        n_points = int(table_cir[ligne + 1][0]) - 1
                        print(n_points)
                        for i in range(n_points):
                            point = []
                            for j in range(3):
                                point.append(
                                    float(table_cir[ligne + 2 + i][j]))
                            mat_points.append(point)
                        dic_face['trous'].append(mat_points)
                        ligne += n_points + 3
            self.cir.append(dic_face)
            face += 1
        sol_file.close()

    def extraire_geom_triangle(self):
        """
        extrait une geometrie triangulee d'un -cir- triangule.
        (ne retrouve pas les sommets des faces mais la liste des triangles
        les composant)
        """
        chrono = Chrono('extraire_geom_triangle', 'solFile')

        points = []
        triangles = []
        faces = []
        normales = []
        n_triangle = 0
        n_face = 0
        cir = self.cir
        print('\t')
        print('\t extraction des contours de %s faces' % len(self.cir))
        #print()
        n_point = 0
        print(' ', end=' ')
        for face in range(len(self.cir)):
            sys.stdout.write('+')
            contours = cir[face]['contours']
            normale = cir[face]['normale']
            ligne_face = []
            for contour in contours:
                normales.append(normale)
                ligne_face.append(n_triangle)
                n_triangle += 1
                triangle = []
                for point in contour:
                    if point not in points:
                        n_point += 1
                        points.append(point)
                    triangle.append(points.index(point) + 1)
                triangles.append(triangle)

            faces.append(ligne_face)
            n_face += 1

        #print()
        #print('\t fin extraction')
        #print('\t %s faces, %s triangles, %s points' % (n_face,
                                                        #n_triangle,
                                                        #n_point))

        self.geom.points = np.array(points)

        self.geom.triangles.points = np.array(triangles)
        self.geom.triangles.normale = np.array(normales)

        self.geom.faces.liste_triangles = faces

        self.geom.n_points = len(self.geom.points)
        self.geom.n_triangles = len(self.geom.triangles.points)
        self.geom.n_faces = len(self.geom.faces.liste_triangles)
        chrono.fin()

    def extraire_geom_triangle_bis(self):
        """
        extrait une geometrie triangulee d'un -cir- triangule.
        (ne retrouve pas les sommets des faces mais la liste des triangles
        les composant)
        """
        chrono = Chrono('extraire_geom_triangle', 'solFile')

        points = []
        triangles = []
        faces = []
        normales = []
        n_triangle = 0
        n_face = 0
        cir = self.cir
        print('=')
        print('=    extraction des contours de %s faces' % len(self.cir))
        print('=')
        n_point = 0
        print('=', end=' ')
        for face in range(len(self.cir)):
            sys.stdout.write('+')
            contours = cir[face]['contours']
            normale = cir[face]['normale']
            ligne_face = []
            for contour in contours:
                normales.append(normale)
                ligne_face.append(n_triangle)
                n_triangle += 1
                triangle = []
                for point in contour:
                    if point not in points:
                        n_point += 1
                        points.append(point)
                    triangle.append(points.index(point) + 1)
                triangles.append(triangle)

            faces.append(ligne_face)
            n_face += 1

        #print('=')
        #print('=   fin extraction')
        #print('=   %s faces, %s triangles, %s points' % (n_face,
        #                                                 n_triangle,
        #                                                 n_point))

        self.geom.points = np.array(points)

        self.geom.triangles.points = np.array(triangles)
        self.geom.triangles.normale = np.array(normales)

        self.geom.faces.liste_triangles = faces

        self.geom.n_points = len(self.geom.points)
        self.geom.n_triangles = len(self.geom.triangles.points)
        self.geom.n_faces = len(self.geom.faces.liste_triangles)
        chrono.fin()

    def extraire_geom_face(self):
        points = []
        faces = []
        normales = []
        trous = []
        n_face = 0
        n_points = 0

        print('\t-> extraction geom face')
        print(len(self.cir))
        for face in range(len(self.cir)):
            # len(self.cir) =nombre de face
            # print self.cir[face]['contours']
            normales.append(self.cir[face]['normale'])
            contour = self.cir[face]['contours'][0]
            face_points = []
            face_trous = self.cir[face]['trous']
            trous_points = []
            # print self.cir[face]['trous']
            for point in contour:
                if point not in points:
                    n_points += 1
                    points.append(point)
                face_points.append(points.index(point) + 1)
            faces.append(face_points)
            for trou in face_trous:
                trou_points = []
                for point in trou:
                    if point not in points:
                        points.append(point)
                    trou_points.append(points.index(point) + 1)
                trous_points.append(trou_points)
            trous.append(trous_points)
            n_face += 1

        self.geom.points = np.array(points)
        self.geom.faces.points = faces
        self.geom.faces.normale = np.array(normales)
        self.geom.faces.liste_trous = trous

        self.geom.n_points = len(self.geom.points)
        self.geom.n_triangles = 0
        self.geom.n_faces = len(self.geom.faces.points)

        print('\t-> %s points et %s faces extraites' %
              (self.geom.n_points, self.geom.n_faces))

    def extraire_geom_face_old(self):
        points = []
        faces = []
        normales = []

        n_face = 0
        n_points = 0

        print('\t-> extraction geom face')

        for face in range(len(self.cir)):
            normales.append(self.cir[face]['normale'])
            contour = self.cir[face]['contours'][0]
            face_points = []
            for point in contour:
                if point not in points:
                    n_points += 1
                    points.append(point)
                face_points.append(points.index(point) + 1)
            faces.append(face_points)
            n_face += 1

        self.geom.points = np.array(points)
        self.geom.faces.points = faces
        self.geom.faces.normale = np.array(normales)

        self.geom.n_points = len(self.geom.points)
        self.geom.n_triangles = 0
        self.geom.n_faces = len(self.geom.faces.points)

        print('\t-> %s points et %s faces extraites' %
              (self.geom.n_points, self.geom.n_faces))

    def importer_geom(self, geom):
        """
        assigne la geometrie
        """
        self.geom = geom

    def exporter(self, bool_face=None, faces=False):
        """
        exporte la geometrie vers dans un fichier -cir-

        possibilite de filtrer les faces avec une liste de booleens:
            bool_face:array(bool) (obsolete)

        face=False -> geometrie triangulee
        face=True  -> geometrie non triangulee
        """

        # appelle des methodes differentes s'il y a une selection par face
        if bool_face is not None:
            fichier = self._exporter_sous_geom_texte(bool_face, faces=faces)
        else:
            fichier = self._exporter_geom_texte(faces=faces)

        #
        ecrire_fichier(self.fname, fichier)

    def _exporter_geom_texte(self, faces=False):
        """
        renvoi la géométrie .cir sous forme d'une chaîne de caractères
        exporte la géométrie triangulée par face
        **ecriture entete
        **pour chaque face i, appelle ecrire_face(i), qui pour chaque contour n,
            appelle ecritureContour(n)
        """

        # appelle une méthode différente si la geometrie doit etre exportee
        # triangulee ou pas
        if not faces:
            ecrire_face = self._ecrire_face_triangulee
        else:
            ecrire_face = self._ecrire_face

        # entete
        fichier = '%s \t %s \n' % (self.geom.n_faces, self.geom.n_faces)\
            + '\t 1 \t 0\n' * 5

        # faces
        for i in range(self.geom.n_faces):
            fichier += ecrire_face(i)

        return fichier

    def _exporter_sous_geom_texte(self, bool_face, faces=False):
        """
        plus ou moins obsolete:
           mieux vaut creer une nouvelle geometrie depuis la geometrie mere
           puis l'exporter en entier...
        """
        liste_face = np.arange(self.geom.n_faces)[bool_face]

        if not faces:
            ecrire_face = self._ecrire_face_triangulee
        else:
            ecrire_face = self._ecrire_face

        fichier = '%s \t %s \n' % (len(liste_face), max(liste_face))\
            + '\t 1 \t 0\n' * 5

        for i in liste_face:
            fichier += ecrire_face(i)

        return fichier

    def _ecrire_face(self, no_face):
        """
        renvoi une chaîne représentant la face non triangulée en -cir-
        """
        no_bis = int(no_face) + 1
        face = 'f%s\t1\n' % no_bis
        face += ecrire_ligne(self.geom.faces.normale[no_face]) + '\n'
        face += self._ecrire_contour(
            self.geom.faces.points[no_face])

        return face

    def _ecrire_face_triangulee(self, no_face):
        """
        renvoi une string représentant la face dans le format .cir
        """
        n_contours = len(self.geom.faces.liste_triangles[no_face])
        liste_triangles = self.geom.faces.liste_triangles[no_face]

        face = 'f%s\t%s\n' % (no_face + 1, n_contours)
        face += ecrire_ligne(self.geom.triangles.normale[
            liste_triangles[0]]) + '\n'

        for triangle in liste_triangles:
            face += self._ecrire_contour(
                self.geom.triangles.points[triangle])
        return face

    def _ecrire_contour(self, points):
        """
        renvoi une chaîne représentant le contour dans le format .cir
        """
        n_points = len(points)
        contour = 'c0\n%s\n' % str(n_points + 1)
        for i in range(n_points):
            contour += '\t' + \
                ecrire_ligne(self.geom.points[points[i] - 1]) + '\n'
        contour += '\t' + ecrire_ligne(self.geom.points[points[0] - 1]) + '\n'

        return contour