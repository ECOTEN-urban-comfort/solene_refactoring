# -*- coding: utf-8 -*-
"""
Created on Wed Sep  8 15:58:19 2010

@author: laurent
"""

import os
import xml.dom.minidom

from infrastructure.solene.utils import ecrire_fichier

# import logging
# logging.basicConfig(filename='myapp.log', level=logging.INFO)
# logger = logging.getLogger(__name__)


def get_data(element_xml, nom):
    """
    retourne la valeur du champ -nom- 
    element_xml : element d'un fichier xml dans lequel le champ nom est unique
    """
    try:
        data = element_xml.getElementsByTagName(nom)[0].firstChild.data
    except:
        print('pas de champ ', nom)
        data = None
    return data

class XmlFile:
    """
    gestion des cas / parametre dans un fichier xml
    """
    def __init__(self, chemin_fichier_xml = None):
        self.chemin_xml = chemin_fichier_xml
        if self.chemin_xml and os.path.isfile(self.chemin_xml):
            self.exist = True
        else:
            self.exist = False
        
    def definir_champ(self, 
                          chemin_noeud, 
                          attribut = None, 
                          valeur_attribut = None,
                          data = None,
			  rang_noeud_enfant=0):
        
        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        
        chemin = chemin_noeud.split('/')
        #if len(chemin) == 1:
            #noeud = doc.firstChild
        
        for rep in chemin:
            try:
                noeud = noeud.getElementsByTagName(rep)[rang_noeud_enfant]
            except:
                noeud_plus = doc.createElement(rep)
                noeud.appendChild(noeud_plus)
                noeud = noeud_plus
        
        if attribut:
            noeud.setAttribute(attribut, valeur_attribut)
        if data:
            try:
                noeud.firstChild.data = data
            except:
                noeud_text = doc.createTextNode(data)
                noeud.appendChild(noeud_text)
     
        ecrire_fichier(self.chemin_xml, doc.toxml())

    def lire_champ(self, chemin_noeud):
        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        
        chemin = chemin_noeud.split('/')        
        for rep in chemin:
            try:
                noeud = noeud.getElementsByTagName(rep)[0]
            except:
                print('pas de champ ', rep)
                
        try:
            data = noeud.firstChild.data
        except:
            print('a marche pas')

        return data

    def supprimer_champ(self, chemin_noeud):
        
        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        
        chemin = chemin_noeud.split('/')
        #if len(chemin) == 1:
            #noeud = doc.firstChild
        
        for rep in chemin[:-1]:
            try:
                noeud = noeud.getElementsByTagName(rep)[0]
            except:
                pass
        a_supprimer = noeud.getElementsByTagName(chemin[-1])[0]
        noeud.removeChild(a_supprimer)

        ecrire_fichier(self.chemin_xml, doc.toxml())
