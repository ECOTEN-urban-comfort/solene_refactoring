# -*- coding: utf-8 -*-
"""
Created on Wed Sep  8 15:58:19 2010

@author: laurent
"""

import os
import xml.dom.minidom

from infrastructure.geometry.legacy.utils import ecrire_fichier

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

def return_interval(fichier_xml):
    """
    renvoi l'interval de simulation sous forme d'une liste
    """
    interval = []
    lst_nom_champ = ['debut_jour',
                 'debut_mois',
                 'debut_heure',
                 'debut_minutes',
                 'fin_jour',
                 'fin_mois',
                 'fin_heure',
                 'fin_minutes',
                 'pas_heure',
                 'pas_minutes']
                 
    for nom_champ in lst_nom_champ:
        interval.append(int(get_data(fichier_xml, nom_champ)))
        
    return interval

class XmlFile:
    """
    Case management / parameters in an XML file
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
                          data = None):
        
        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        
        chemin = chemin_noeud.split('/')
        #if len(chemin) == 1:
            #noeud = doc.firstChild
        
        for rep in chemin:
            try:
                noeud = noeud.getElementsByTagName(rep)[0]
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

    def ajouter_noeud(self, noeud_parent, noeud_enfant):
        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        chemin = noeud_parent.split('/')
        for rep in chemin:
            try:
                noeud = noeud.getElementsByTagName(rep)[0]
            except:
                print('pas de noeud %s' %rep)
        n = doc.createElement(noeud_enfant)
        noeud.appendChild(n)
        ecrire_fichier(self.chemin_xml, doc.toxml())
        

    def parse_constant(self, noeud_constant):
        attr = {}
        for cle, it in list(noeud_constant.attributes.items()):
            attr[cle] = it
        attr['value'] = noeud_constant.firstChild.data
        if 'type' in attr:
            if attr['type'] == 'float':
                attr['value'] = float(attr['value'])
            elif attr['type'] == 'bool':
                attr['value'] = bool(attr['value'])
            elif attr['type'] == 'int':
                attr['value'] = int(attr['value'])
        return attr

    def parse_list(self, noeud_liste):
        """
        recupere la liste des noeuds nommes "constant" dans le noeud
        -noeud_liste-
        
        renvoi un dictionnaire avec les cles corerspondantes a l'attribut 
        "name" et contenant tous les attributs et la valeur (cle "value")
        """
        dic_liste = {}

        doc = xml.dom.minidom.parse(self.chemin_xml)        
        noeud = doc
        chemin = noeud_liste.split('/')
        for rep in chemin:
            try:
                noeud = noeud.getElementsByTagName(rep)[0]
            except:
                print('pas de noeud %s' %rep)
        liste = noeud.getElementsByTagName('constant')
        for el in liste:
            el_attr = self.parse_constant(el)
            dic_liste[el_attr['name']] = el_attr
        return dic_liste
