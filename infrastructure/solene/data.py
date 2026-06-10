#!/usr/bin/python
# -*- coding: utf-8 -*-

## @package data
#Définition d'une classe Data

"""
--- Version 0.01 ---

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr


"""
from numpy import *
from infrastructure.solene.geom import *
from infrastructure.solene.utils import *
from infrastructure.solene.vtkFile import *


def enlever_nan(donnee):
    """
    remplace les -nan- par des 0 dans -donnee-
    utiliser pour réparer les boulettes de solene qui cree des nan la nuit
    """
    for i in range(len(donnee)):
        val = donnee[i]
        if str(val) == 'nan':
            print(val)
            donnee[i] = 0
            
    return donnee

class Data:
    def __init__(self, nom='', geom = None, type = '2D'):
        self.nom = nom
        self.fname = ''
        if geom:
            self.geom = geom
            self.definir_type(type = type)
        else:
            self.geom = Geom()
            self.n_elem = 0
            
        self.type = ''
        self.liste_ts = []
        self.data = {}

    def definir_type(self, type='2D'):

        self.type = type
        if type == '2D':
            self.n_elem = self.geom.n_triangles
        elif type == '3D':
            self.n_elem = self.geom.n_tetras
        else :
            print('type de donnee = 1D ou 2D')
            

    def ajouter_donnee(self, t, donnee, nom_donnee):
        if len(donnee) == self.n_elem:
            if 'NaN' in donnee.tolist():
                print(t, nom_donnee)
                donnee = enlever_nan(donnee)
            try:
                self.data[t][nom_donnee]=donnee
            except:
                self.data[t] = {}
                self.data[t][nom_donnee]=donnee
                
        else :
            print('la longueur ne correspond pas : \n\t nElem=%s, len(donnee)=%s' % (self.nElem, len(donnee)))

    def exporter_vtk(self, nom_fichier, liste_variable = None):
        if liste_variable :
            liste_v = liste_variable
        else : 
            liste_v = list(self.data[0].keys())
            
        for ts in list(self.data.keys()):
            vtk = VtkFile(nom_fichier + '_' + str(ts) +'.vtu', geom=self.geom)
            vtk.ecrire_vtu()
            for var in liste_v:
                vtk.ajouter_donnee(self.data[ts][var], var) 
            vtk.close_xml()
            
    def calculer_evolution_moyenne(self, variable):
        sortie = []
        for t in range(len(list(self.data.keys()))):
            sortie.append(self.data[t][variable].mean())
        return float32(sortie)
    
    def extraire_evolution_triangle(self, variable, i_triangle):
        sortie = []
        for t in range(len(list(self.data.keys()))):
            sortie.append(self.data[t][variable][i_triangle])
        return float32(sortie)
        
    def extraire_evolution_triangle_multi(self, liste_variable, i_triangle):
        sortie = {}
        for var in liste_variable:
            sortie[var] = self.extraire_evolution_triangle(var, i_triangle)
            
        return sortie

    def extraire_evolution_triangles(self, lst_variables, lst_triangles):
        sortie = {}
        for i in lst_triangles:
            for var in lst_variables:
                sortie['%s_%s'%(i,var)] = self.extraire_evolution_triangle(i, var)
        return sortie

    def extraire_monitoring(self, i_triangle):
        
        return 
    
#class Resu:
