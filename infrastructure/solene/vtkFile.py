#!/usr/bin/python
# -*- coding: utf-8 -*-

"""@package vtkFile
Définition de la classe VtkFile

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr
"""
import xml.dom.minidom
from infrastructure.solene.geom import *

def exporter_vtu(nom_fichier, geom, liste_donnee, liste_nom_donnee):
    vtk = VtkFile(nom_fichier)
    vtk.geom = geom
    vtk.ecrire_vtu()
    
    if not len(set(liste_nom_donnee)) == len(liste_nom_donnee):
        print("=! ne la as utiliser le meme nom deux fois")
    
    for i in range(liste_donnee):
        vtk.ajouter_donnee(liste_donnee[i], liste_nom_donnee[i])
        
    vtk.close_xml()

class VtkFile:
    """
    Classe de fichier vtk
    pour des geometrie de type 'unstructured grid'
    
    permet d'exporter des resultats dans le logiciel de visualisation 3d
    paraView
    deux formats possibles : ascii pure (.vtk) et xml (.vtu)
    rq : ascii obsolete, utiliser vtu
    
    methode xml : 
        vtk = VtkFile(nom_fichier)
        vtk.geom = geom
        vtk.ecrire_vtu()
        vtk.ajouter_donnee(donnee, nom_donnee)
        vtk.close_xml()
    """
    def __init__(self, nom, geom = None):

        self.nom = nom
        self.doc = xml.dom.minidom.Document()
        
        if geom:
            self.geom = geom
            self.ecrire_vtu()
        else :
            self.geom = Geom()
        self.vtk_texte = ''
        
        self.n_cells = 0

    def ecrire_vtu(self, volum = False):
        """
        ecriture de la geometrie au format .vtu (xml)
        
        """
        ##########
        # Creation des arrays....
        #
        # liste des coordonnees des points à la suite :
        # liste des coordonnees des points à la suite :
        # 'x1 y1 z1 x2 y2 .... yn zn'
        points_string = ''
        for point in self.geom.points:
            for coord in point:
                points_string += '%s ' % str(coord)

        self.points_string = points_string
        # liste des points par cellule à la suite :
        # '1a 1b 1c 2a 2b 2c ...'
        cells_conn_string = ''
        cells_types_string = ''
        cells_offset_string = ''
        cell_pos = 0
        cells_number = self.geom.n_triangles

        # elements surfaciques:

        cells_types_string = '5 ' * self.geom.n_triangles
        for triangle in self.geom.triangles.points:
            for point in triangle:
                cells_conn_string += '%s ' % str(point-1)
                cells_types_string += '%s ' % str(5)
                cell_pos += 3
                cells_offset_string += '%s ' % cell_pos
                
        

        # on ajoute les volumes si c'est la peine
        if volum:
            cells_types_string += '10 ' *self.geom.n_tetras
            cells_number += self.geom.n_tetras
            for tetra in self.geom.tetras.points:
                for point in tetra:
                    cells_conn_string += '%s '      % str(point-1)
                    cell_pos += 4
                    cells_offset_string += '%s ' % cell_pos

        ##########
        # Remplissagle du xml
        #
        # Document and root element
        root_element = self.doc.createElementNS("VTK", "VTKFile")
        root_element.setAttribute("type", "UnstructuredGrid")
        root_element.setAttribute("version", "0.1")
        root_element.setAttribute("byte_order", "LittleEndian")
        self.doc.appendChild(root_element)

        # Unstructured grid element
        self.unstructured_grid = self.doc.createElementNS("VTK", 
                                                    "UnstructuredGrid")
        root_element.appendChild(self.unstructured_grid)

        # Piece 0 (only one)
        piece = self.doc.createElementNS("VTK", "Piece")
        piece.setAttribute("NumberOfPoints", str(self.geom.n_points))
        piece.setAttribute("NumberOfCells", str(cells_number))
        self.unstructured_grid.appendChild(piece)
        
        points = self.doc.createElementNS("VTK", "Points")
        piece.appendChild(points)        

        point_coords = self.doc.createElementNS("VTK", "DataArray")
        point_coords.setAttribute("type", "Float32")
        point_coords.setAttribute("format", "ascii")
        point_coords.setAttribute("NumberOfComponents", "3")
        points.appendChild(point_coords)
                                    
        point_coords_data = self.doc.createTextNode(self.points_string)
        point_coords.appendChild(point_coords_data)   

        #### Cells ####
        cells = self.doc.createElementNS("VTK", "Cells")
        piece.appendChild(cells)        

        # Cell locations
        cell_connectivity = self.doc.createElementNS("VTK", "DataArray")
        cell_connectivity.setAttribute("type", "Int32")
        cell_connectivity.setAttribute("Name", "connectivity")
        cell_connectivity.setAttribute("format", "ascii")        
        cells.appendChild(cell_connectivity)

        # Cell location data
        connectivity = self.doc.createTextNode(cells_conn_string)
        cell_connectivity.appendChild(connectivity)     

        # Cell_offsets
        cell_offsets = self.doc.createElementNS("VTK", "DataArray")
        cell_offsets.setAttribute("type", "Int32")
        cell_offsets.setAttribute("Name", "offsets")
        cell_offsets.setAttribute("format", "ascii")                
        cells.appendChild(cell_offsets)

        offsets = self.doc.createTextNode(cells_offset_string)
        cell_offsets.appendChild(offsets)

        # Cell type
        cell_types = self.doc.createElementNS("VTK", "DataArray")
        cell_types.setAttribute("type", "Int8")
        cell_types.setAttribute("Name", "types")
        cell_types.setAttribute("format", "ascii")        
        cells.appendChild(cell_types)

        types = self.doc.createTextNode(cells_types_string)
        cell_types.appendChild(types)  


        cell_data = self.doc.createElementNS("VTK", "CellData")
        piece.appendChild(cell_data)

    def ajouter_donnee(self, donnee, nomdonnee):
        """
        ecriture d'une donnee dans le xml
        """
        donnee_string = ''
        for i in donnee:
            donnee_string += '%s ' % i
            
        donnee_string = donnee_string.replace('nan', '0')
                
        cell_data = self.doc.childNodes[0].childNodes[0].childNodes[0].childNodes[2]
        cell_data.setAttribute("Scalars", nomdonnee)
        
        data = self.doc.createElementNS("VTK", "DataArray")
        data.setAttribute("type", "Float32")
        data.setAttribute("Name", nomdonnee)
        data.setAttribute("format", "ascii")  
        cell_data.appendChild(data)

        donnee_data = self.doc.createTextNode(donnee_string)
        data.appendChild(donnee_data)

    def close_xml(self):
        """
        exporte le xml cree dans le fichier -self.nom-
        """
        print('creation %s' % self.nom)
        # Write to file and exit
        out_file = open(self.nom, 'w')
        # xml.dom.ext.PrettyPrint(doc, file)
        self.doc.writexml(out_file, newl='\n')
        out_file.close()