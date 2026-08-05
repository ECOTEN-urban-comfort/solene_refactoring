#!/usr/bin/python
# -*- coding: utf-8 -*-

"""@package satCommand
Définition d'une classe SatCommand de contrôle 
des simulations de Saturne

Version 0.02
04/04/2013

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr

Modified in 5/14/2026
    farid almaulaui, Ecoten urban comfort
    faridmaulaui@gmail.com
"""

import os
from pathlib import Path
import time
import xml.dom.minidom

import subprocess
import numpy as np

from infrastructure.solene.utils import ecrire_fichier, lst2str, couper_ligne_f77, couper_ligne_cpp
from infrastructure.solene.xmlFile import XmlFile
from infrastructure.saturne.templates import SaturneTemplateRepository

import threading
import queue
from typing import Callable, Any



JOIN = os.path.join

PARAM_METEO = ['T', 'v', 'direction', 'w']



##############################
# Class helper functions
##############################

def parser_listing(fichier_listing):
    """
    parses the listing file and returns a list of dictionaries,
    each containing information related to a time step.
    """
    
    listing = open(fichier_listing,'r')
    listing_lignes = listing.readlines()
    time_step = 0
    liste_ts = []
    dic_ts = {}
    for ligne in listing_lignes:
        tab = ligne.split()
                
        if len(tab)>0 and tab[0] == 'INSTANT':
            if time_step != 0:
                liste_ts.append(dic_ts)
            dic_ts = {}
            time_step +=  1
            dic_ts['TimeStep'] = time_step
                
        elif len(tab)>0 and tab[0] == 'c':
            dic = parse_listing_tab(ligne)

                        
            var = dic['Variable'].split()
            if len(var) == 1:
                dic_var = var[0]
            else:
                dic_var = ''
                for i in range(len(var)):
                    dic_var +=  str(var[i])
            dic['Variable'] = dic_var
            dic_ts['conv_'+dic['Variable']] = dic

        elif len(tab)>1 and tab[0] == 'v':
            dic = parse_listing_tab(ligne)
            dic_ts[dic['Variable']] = dic

        elif len(tab)>6 and tab[0] == 'CPU':
            dic_ts['CPU time'] = float(tab[7])

        elif 'END OF CALCULATION' in ligne:
            print('finished calculation')
            
    return liste_ts
    
def parse_listing_lines(new_lines, timestep):
    """
    Listing parsing function (the third one)
    which works with SatCommand.follow_simulation()
    """
    sim_continu = True
    new_step = False
    lst_retour = []
    dic_ts = {}
    for ligne in new_lines:
        tab  =  ligne.split()
        
        if len(tab)>0 and tab[0] == 'INSTANT':
            if timestep > 0:
                lst_retour.append(dic_ts)
            dic_ts = {}
            timestep +=  1
            dic_ts['TimeStep'] = timestep
            new_step  =  True

        elif len(tab)>0 and tab[0] == 'c':
            dic = parse_listing_tab(ligne)

            var = dic['Variable'].split()
            if len(var) == 1:
                dic_var = var[0]
            else:
                dic_var = ''
                for i in range(len(var)):
                    dic_var +=  str(var[i])
            dic['Variable'] = dic_var
            dic_ts['conv_'+dic['Variable']] = dic

        elif len(tab)>1 and tab[0] == 'v':
            dic = parse_listing_tab(ligne)
            dic_ts[dic['Variable']] = dic

        elif len(tab)>7 and tab[0] == 'CPU':
            dic_ts['CPU time'] = float(tab[7])

        elif 'END OF CALCULATION' in ligne:
            sim_continu = False
            
    return lst_retour, new_step, sim_continu

def parse_listing_tab(ligne):
    """
    Parses array rows (convergence, variable)
    Returns a dictionary
    Parses CPU TIME rows
    Returns a dim2 vector
    Parses INSTANT rows
    Returns an integer
    """
    tab = ligne.split()
    
    if len(tab) == 0:
        print('Error: Empty line!')
        dic_ligne = None

    if tab[0] == 'c':
        dic_ligne = {}
        dic_ligne['Variable'] = str(ligne[3:15])
        dic_ligne['Rhs norm'] = float(ligne[15:27])
        dic_ligne['N_iter'] = int(ligne[28:35])
        try:
            dic_ligne['Norm. residual']  =  float(ligne[37:49])
        except ValueError:
            dic_ligne['Norm. residual']  =  str(ligne[37:49])

        try:
            dic_ligne['derive']  =  float(ligne[51:63])
        except ValueError:
            dic_ligne['derive']  =  str(ligne[51:63])

    elif tab[0] == 'v':
        dic_ligne = {}

        # Everything except the last 4 values = variable name
        if len(tab) >= 6:
            var_parts = tab[1:-4]
            values = tab[-4:]
        else:
            # fallback (in case of weird lines)
            var_parts = tab[1:2]
            values = tab[2:]

        dic_ligne['Variable'] = ' '.join(var_parts)

        try:
            dic_ligne['Min value'] = float(values[0])
        except:
            dic_ligne['Min value'] = None

        try:
            dic_ligne['Max value'] = float(values[1])
        except:
            dic_ligne['Max value'] = None

        if len(values) > 2:
            dic_ligne['Min clip'] = float(values[2])
        if len(values) > 3:
            dic_ligne['Max clip'] = float(values[3])

    elif tab[0] == 'CPU':
        dic_ligne = [int(tab[6]), float(tab[7])]
    elif tab[0] == 'INSTANT':
        dic_ligne = tab[5]
    else:
        dic_ligne = None
        print("Error: Cannot parse this line!")

    return dic_ligne

##############################
# Saturn simulation control class
##############################

class SatCommand:
    def __init__(
        self,
        chemin_sim: str | Path,
        nom_cas: str,
        *,
        code_saturne_executable: str | Path,
        tmp_saturne_dir: str | Path,
        template_repo: SaturneTemplateRepository | None = None,
        creer: bool = False,
        case: str = "CASE1",
    ) -> None:

        self.nom_cas = nom_cas.upper()
        self.case1 = case
        self.code_saturne_executable = Path(code_saturne_executable)
        self.template_repo = template_repo or SaturneTemplateRepository()

        self.tmp_saturne_dir = (
            Path(tmp_saturne_dir)
            if tmp_saturne_dir is not None
            else chemin_sim.parent / "tmpSaturne"
        )
        self.tmp_saturne_dir.mkdir(parents=True, exist_ok=True)

        # paths to Saturn's output  file tree
        self.chemins = {}
        self.chemins['sim'] = chemin_sim
        self.chemins['case'] = os.path.abspath(JOIN(self.chemins['sim'], self.nom_cas.upper()))
        self.chemins['mesh'] = JOIN(self.chemins['case'], 'MESH')
        self.chemins['data'] = JOIN(self.chemins['case'], self.case1, 'DATA')
        self.chemins['resu'] = JOIN(self.chemins['case'], self.case1 ,'RESU')
        self.chemins['src'] = JOIN(self.chemins['case'], self.case1, 'SRC')
        
        self.chemins['echange'] = JOIN(self.chemins['resu'], 'echange')
        
        self.chemins['conf_xml'] = JOIN(self.chemins['data'], 'param.xml')
        self.chemins['tmp'] = ''
        self.chemins['listing'] = None        

        self.chemins['geometrie'] = ''
        
        # path to weather files
        self.param_meteo = PARAM_METEO
        
        # solene output file naming for coupling
        self.nom_dat = {}
        self.nom_dat['Tair'] = JOIN(self.chemins['echange'], 'temperature')
        self.nom_dat['Vair'] = JOIN(self.chemins['echange'], 'vitesse')
        self.nom_dat['h_conv'] = JOIN(self.chemins['echange'], 'h_conv')
        self.nom_dat['hs'] = JOIN(self.chemins['echange'],'hs')
        self.nom_dat['Ts'] = JOIN(self.chemins['data'],'T_mur')
        self.nom_dat['Flatent'] = JOIN(self.chemins['data'], 'Flatent')
        self.nom_dat['Fsensible'] = JOIN(self.chemins['data'], 'Fsensible')

        # managing listings and tracking simulations
        self.conf_xml = XmlFile()

        self.simu = None
        self.start_time = ''
        self.restart = ''    
        
        self.convergence_vitesse = []
        self.timestep = []
        self.total_lignes = [0, 0]

        self.liste_timestep = []
        self.fichierlignes = []
        self.trace = 1

        self.processors_number = 1
        
        self.iteration = 200
        
        # Code Saturne subroutines
        self.cs_boundary_perso = self.template_repo.load().cs_user_boundary_conditions
        self.cs_sourceterms_perso = self.template_repo.load().cs_user_source_terms
        self.cs_postprocess_perso = self.template_repo.load().cs_user_postprocessing
        # Code Saturne Setup/Config XML
        self.saturne_xml_perso = self.template_repo.load().saturne_xml
        # definition des familles pour les subroutines
        self.sat_familles = {}

        if creer or not os.path.isdir(self.chemins['case']):
            self.creer_cas()



    def trouver_xml(self, nom_xml = None):
        """
        cherche le fichier de configuration xml dans le repertoire -data-
        demande une intervention de l'utilisateur sur plusieurs fichiers xml 
        sont présents
        """
        if not nom_xml:
            liste_fichier_data = os.listdir(self.chemins['data'])
            liste_xml = []
            for fichier in liste_fichier_data:
                if fichier[-4:] == '.xml':
                    liste_xml.append(fichier)
            if len(liste_xml) == 1:
                nom_xml = liste_xml[0]
            elif len(liste_xml) == 0:
                print('no xml files in -DATA-')

            else : 
                print('several xml files in -DATA-: ')
                for i in range(len(liste_xml)):
                    print('\t', i+1, liste_xml[i])
                chemin_user = str(input("Name or number?"))
                try:
                    chemin_user = int(chemin_user)
                    nom_xml = liste_xml[int(chemin_user)-1]
                except ValueError:
                    if nom_xml in liste_xml:
                        nom_xml = chemin_user

        if nom_xml:                            
            self.chemins['conf_xml'] = JOIN(self.chemins['data'], nom_xml)
            self.conf_xml = XmlFile(self.chemins['conf_xml'])
        else:
            self.chemins['conf_xml'] = None

    def creer_cas(self):
        """
        creation du cas s'il n'existe pas : 
            repertoire principale
            + toute l'arborescente avec l'executable : code_saturne create
        """
        
        ici = os.path.realpath(os.path.curdir)
        try:
            if not os.path.isdir(self.chemins['sim']):
                os.mkdir(self.chemins['sim'])
        except OSError:
            print('\t ! probleme creation repertoire -simulsat-')

        if not os.path.isdir(self.chemins['mesh']):
            os.chdir(self.chemins['sim'])
            subprocess.call([self.code_saturne_executable, 'create', '-s', self.nom_cas])
            os.makedirs(self.chemins['echange'])
            
        else:
            liste_mesh = os.listdir(self.chemins['mesh'])
            liste_mesh_med = []
            for mesh in liste_mesh:
                if mesh[-4:] == '.med':
                    liste_mesh_med.append(mesh)
            if len(liste_mesh_med) == 1:
                self.chemins['geometrie'] = JOIN(self.chemins['mesh'], 
                                                 liste_mesh_med[0])
        os.chdir(ici)

    def copier_geometrie(self, chemin_geometrie, check = False):
        """
        copie la geometrie -chemin_geometrie- dans le répertoire -MESH-
        de l'arborescence de -simulSat-
        """
        print('\t copier la geometrie dans MESH')
        self.chemins['geometrie'] = JOIN(self.chemins['mesh'], 
                                         os.path.split(chemin_geometrie)[1])

        mesh_exist = os.path.isfile(self.chemins['geometrie'])

        if mesh_exist:
            print("\t -> geometrie med deja présente dans -simulSat-")
            print("\t -> le cas SATURNE est deja cree")
            if check:
                ecraser = str(input("\t -> écraser (o/n)?"))
            else:
                ecraser = 'n'
                
            if ecraser == 'o' or ecraser == 'y':
                ecraser = True
            else:
                ecraser = False
        
        if not mesh_exist or ecraser :
            
            print('\t -> SHELL: "cp %s\n\t %s"' % (chemin_geometrie, 
                                                   self.chemins['mesh']))
            com = ['cp', 
                   chemin_geometrie, 
                   self.chemins['mesh']]
            subprocess.call(com)
        
        nom_geometrie = chemin_geometrie.split('/')[-1]
        self.chemins['geometrie'] = self.chemins['mesh']+'/'+ nom_geometrie

    def definir_meteo(self, dic_meteo):
        """
        exports weather files by variable
        """
        for param in self.param_meteo:
            if param == 'v' and dic_meteo[param] == 0:
                val_param = 1.5
            elif param == 'w':
                val_param = dic_meteo[param]/1000
            else:
                val_param = dic_meteo[param]
            chemin_fichier = JOIN(self.chemins['data'], param)
            ecrire_fichier(chemin_fichier, str(float(val_param)))

    def definir_param_meteo(self, nom_param, valeur):
        """
        ecrit la valeur dans le fichier meteo de -nom_param-
        """
        chemin_fichier = JOIN(self.chemins['data'], nom_param)
        ecrire_fichier(chemin_fichier, str(valeur))

    def recuperer_listes_from_famille(self, famille):
        """
        Create the families dictionary needed by boundary conditions file / CS in general
        """
        self.sat_familles = famille.exporter_familles_saturne()


    def creer_cs_boundary(self):
        """
        Create the boundary condition file, complete with:
        - the path to the DATA directory
        - the names of the corresponding families (sat_inlet and murs)
        """
        self.cs_boundary_perso = self.cs_boundary_perso.replace('$$chemin_data', couper_ligne_f77(self.chemins['data']))

        self.cs_boundary_perso = self.cs_boundary_perso.replace('$$familles_sat_inlet', couper_ligne_f77(lst2str(self.sat_familles['sat_inlet'])))

        str_fam_murs = lst2str(self.sat_familles['murs']+self.sat_familles['sat_wall'])
        self.cs_boundary_perso = self.cs_boundary_perso.replace('$$familles_murs', couper_ligne_f77(str_fam_murs))
        
        self.cs_boundary_perso = self.cs_boundary_perso.replace('$$familles_sat_sym', lst2str(self.sat_familles['sat_sym']))


    def creer_cs_postprocess(self):
        """
        Provides:
        outputs the following .dat files in the echange directory
        temperature, vitesse(windspeed), h_conv, hs
        """
        cs_postprocess_perso = self.cs_postprocess_perso
        cs_postprocess_perso = cs_postprocess_perso.replace('$$chemin_echange', couper_ligne_cpp(self.chemins['echange']))
        cs_postprocess_perso = cs_postprocess_perso.replace('$$familles_murs', couper_ligne_cpp(lst2str(self.sat_familles['murs'])))
        cs_postprocess_perso = cs_postprocess_perso.replace('$$familles_surface_arbre',  couper_ligne_cpp(lst2str(self.sat_familles['surface_arbre'])))
                                            
        self.cs_postprocess_perso = cs_postprocess_perso

    def creer_cs_sourceterms(self):
        """
        sets up the physics needed for code saturne to run 
        """
        self.cs_sourceterms_perso = self.cs_sourceterms_perso.replace('$$chemin_data', couper_ligne_cpp(self.chemins['data']))

        self.cs_sourceterms_perso = self.cs_sourceterms_perso.replace('$$familles_volume_arbre', lst2str(self.sat_familles['volume_arbre']))

        self.cs_sourceterms_perso = self.cs_sourceterms_perso.replace('$$familles_murs', couper_ligne_cpp(lst2str(self.sat_familles['murs'])))

        self.cs_sourceterms_perso = self.cs_sourceterms_perso.replace('$$familles_surface_arbre', couper_ligne_cpp(lst2str(self.sat_familles['surface_arbre'])))
                                            
    def ecrire_subroutines(self):
        """
        Create the different user routines
        by specifying the families
        """
        self.creer_cs_boundary()
        nom_cs_boundary = JOIN(self.chemins['src'], 'cs_user_boundary_conditions.f90')
        ecrire_fichier(nom_cs_boundary, self.cs_boundary_perso)
        
        self.creer_cs_sourceterms()
        nom_cs_sourceterms = JOIN(self.chemins['src'], 'cs_user_source_terms.cpp')
        ecrire_fichier(nom_cs_sourceterms, self.cs_sourceterms_perso)

        self.creer_cs_postprocess()
        nom_cs_postprocess = JOIN(self.chemins['src'], 'cs_user_postprocess.cpp')
        ecrire_fichier(nom_cs_postprocess, self.cs_postprocess_perso)

	
    def launch_simulation(self, terminal=True):
        """
        Launches the Saturne simulation.
        """
        print("\033[91m CALLED LANCER \033[0m")

        ici = os.path.realpath(os.curdir)

        start_time = "%d%.2d%.2d-%.2d%.2d" % time.localtime()[:5]

        if start_time == self.start_time:
            while "%0.2d" % time.localtime()[4] == start_time[-2:]:
                time.sleep(1)

        self.start_time = "%d%.2d%.2d-%.2d%.2d" % time.localtime()[:5]

        try:
            os.chdir(self.chemins["data"])

            nom_param = os.path.split(
                self.chemins["conf_xml"]
            )[1]

            com = [
                self.code_saturne_executable,
                "run",
                "--nprocs",
                str(self.processors_number),
                "-p",
                nom_param,
            ]

            self.simulation_error = None

            if terminal:
                # Output is displayed in the current Docker terminal.
                self.simu = subprocess.Popen(com)

            else:
                self.simu = subprocess.Popen(
                    com,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                )

                threading.Thread(
                    target=self._monitor_simulation_output,
                    daemon=True,
                ).start()

        finally:
            os.chdir(ici)

    def _monitor_simulation_output(self):
        errors_list = [ "Error in calculation stage.", "MPI_ABORT", "Run failed in compile or link stage."]

        for line in iter(self.simu.stdout.readline, ''):
            print(f"[SATURNE] {line}", end='')  # keep output visible

            line_lower = line.lower()

            for err in errors_list:
                if err.lower() in line_lower:
                    print(f"\nCODE SATURNE ERROR DETECTED\n")
                    self.simulation_error = err
                    return
            
        self.simu.stdout.close()

    def lire_listing(self):
        """
        reads and parses the listing file
        """
        if not self.chemins['listing']:
            self.chemins['listing'] = JOIN(self.chemins['resu'], self.start_time, 'listing')
        listing_fichier = open(self.chemins['listing'])
        listing_lignes = listing_fichier.readlines()
        listing_fichier.close()
        
        self.liste_timestep = self.parser_listing(listing_lignes)

    def parser_listing(self, listing_lignes):
        """
        parses the listing file and returns a list of dictionaries,
        each containing information related to a time step.
        """
        time_step = 0
        liste_ts = []
        dic_ts = {}
        for ligne in listing_lignes:
            tab = ligne.split()
                    
            if len(tab)>0 and tab[0] == 'INSTANT':
                if time_step != 0:
                    liste_ts.append(dic_ts)
                dic_ts = {}
                time_step +=  1
                dic_ts['TimeStep'] = time_step
                
            elif len(tab)>0 and tab[0] == 'c':
                dic = parse_listing_tab(ligne)

                var = dic['Variable'].split()
                if len(var) == 1:
                    dic_var = var[0]
                else:
                    dic_var = ''
                    for i in range(len(var)):
                        dic_var +=  str(var[i])
                dic['Variable'] = dic_var
                dic_ts['conv_'+dic['Variable']] = dic

            elif len(tab)>1 and tab[0] == 'v':
                dic = parse_listing_tab(ligne)
                dic_ts[dic['Variable']] = dic

            elif len(tab)>6 and tab[0] == 'CPU':
                dic_ts['CPU time'] = float(tab[7])

            elif 'END OF CALCULATION' in ligne:
                print('fin du calcul')
                self.trace = 0
        return liste_ts

    def follow_simulation(self,time_simul = None, fn_nv_timestep = None):
        """
        Extracts the data from the listing file
        on the fly and plots
        at each new time step, executes the fn_nv_timestep() function
        """
        if time_simul:
            rep_tmp = '%s.%s.%s' % (self.nom_cas, self.case1, time_simul)
            self.chemins['tmp'] = JOIN(
                self.chemins["tmp"],
                rep_tmp,
            )

        self.chemins['listing'] = JOIN(self.chemins['resu'], self.start_time, 'listing')
        
        # waits for the 'listing' file to exist and to be complete (fix of bug no.1)
        start_waiting = time.time()
        loops = 0
        while not os.path.exists(self.chemins['listing']):
            # checks for any issues in code saturns output
            self._raise_simulation_error()
            loops += 1
            if loops == 1:
                print("Waiting for the 'listing' file to appear...")
            time.sleep(1)
            if loops % 1800 == 0:  # approx. 30 minutes
                el_time = time.time() - start_waiting
                print("Waiting for the 'listing' file to appear for: ", el_time / 60, " min.")

        loops = 0
        wait = True
        start_waiting_2 = time.time()
        while wait:
            with open(self.chemins['listing'], 'r') as opened_file:
                whole_file_as_string = opened_file.read()                
                if "END OF CALCULATION" in whole_file_as_string:  # "Total memory used:"
                    del whole_file_as_string
                    # time.sleep(1)
                    wait = False
            if wait:
                if loops == 0:
                    print("Waiting for the 'listing' file to be complete...")
                loops += 1
                time.sleep(2)
                if loops % 900 == 0:  # approx. 30 minutes
                    el_time = time.time() - start_waiting_2
                    print("Waiting for the 'listing' file to be complete for: ", el_time / 60, " min.")

        el_time = time.time() - start_waiting
        if el_time < 60:
            print(f"Total time waiting for the 'listing' file was: {el_time:.1f} sec.")
        else:
            print(f"Total time waiting for the 'listing' file was: {el_time / 60:.2f} min.")


        file = open(self.chemins['listing'], 'r')
        new_step = False
        timestep = 0
        xxx = True
        lst_dic_ts = []
        while xxx:
            new_lines = file.readlines()
            new_step = False
            nv_dic, new_step, xxx = parse_listing_lines(new_lines, timestep)
            if len(new_lines)==0:
                time.sleep(10)
            for dic_ts in nv_dic:
                lst_dic_ts.append(dic_ts)
            if new_step and timestep > 1:
                print(timestep)
                if fn_nv_timestep:
                    fn_nv_timestep()

    def _raise_simulation_error(self):
        """
        error handling for code saturne where you can check where the error
        is happening may be expanded in future where is would print the error directly from 
        the code saturne output
        """
        if self.simulation_error is None:
            return

        if self.simulation_error == "Run failed in compile or link stage.":
            path = JOIN( self.chemins['resu'], self.start_time,"compile.log")
            raise RuntimeError( f"{self.simulation_error}\n" f"Check compile log:\n{path}")

        elif self.simulation_error == "error in calculation":
            path = JOIN(self.chemins['resu'], self.start_time, "run_solver")
            raise RuntimeError(  f"{self.simulation_error}\n"  f"Check run_solver log:\n{path}")
        
        else:
            print(" Issue with pc, docker, consult the current developer of solene\n")
            raise RuntimeError(self.simulation_error)

    ### CONFIGURATION SATURNE
    # functions used in main.py to initialise code saturn
    def creer_param_xml(self, case = None, study = None, mesh = None):
        """
        Creates the param xml file
        """
        if not case:
            case = self.case1
        if not study:
            study = self.nom_cas
        if not mesh:
            if self.chemins['geometrie'] != '':
                mesh = os.path.split(self.chemins['geometrie'])[1]
        
        f_saturne_xml = open(self.chemins['conf_xml'], 'w')
        f_saturne_xml.write(self.saturne_xml_perso)
        f_saturne_xml.close()
        self.conf_xml = XmlFile(self.chemins['conf_xml'])
        self.conf_xml.definir_champ('Code_Saturne_GUI', attribut = 'case', valeur_attribut = case)
        self.conf_xml.definir_champ('Code_Saturne_GUI', attribut = 'study', valeur_attribut = study)
        self.conf_xml.definir_champ('Code_Saturne_GUI/solution_domain/meshes_list/mesh',
                                    attribut = 'name',
                                    valeur_attribut = mesh)
        self.conf_xml.definir_champ('Code_Saturne_GUI/solution_domain/meshes_list/mesh',
                                    attribut = 'reorient',
                                    valeur_attribut = 'on')


    def change_num_processers(self, nb_processeurs):
        self.processors_number = nb_processeurs
  
    def change_num_iteration(self, iteration = None):
        """
        modifies the number of iterations in the XML configuration file
        """        
        if iteration:
            self.iteration = iteration

        champ_iterations = 'analysis_control/time_parameters/iterations'

        self.conf_xml.definir_champ(champ_iterations,data = str(self.iteration))

    def ajouter_iterations(self, iteration_plus):
        """
        add iterations to the XML configuration file
        """

        champ_iterations = 'analysis_control/time_parameters/iterations'

        
        iteration = int(self.conf_xml.lire_champ(champ_iterations))
        self.iteration = iteration + iteration_plus
        self.change_num_iteration()
        
        print('\t%s iterations ajoutees,' % iteration_plus, end=' ')
        print('total : %s iterations' % self.iteration)



    def definir_restart(self, restart = None):
        """
        utilise la fonction restart de saturne pour commencer la prochaine 
        simulation a partir des resultats d'une simulation precedente

        pour code_saturne gen3
        """

        if restart:
            if len(restart) == 13:
                ch_restart = JOIN(self.chemins['resu'], restart, 'checkpoint')
            else:
                ch_restart = JOIN(restart,'checkpoint')
        else:
            ch_restart = JOIN(self.chemins['resu'], self.start_time, 'checkpoint')

        if not os.path.exists(ch_restart):
            print('ch_restart n existe pas')
            return 1

        champ_restart = 'Code_Saturne_GUI/calculation_management/start_restart/restart'
        doc = xml.dom.minidom.parse(self.chemins['conf_xml'])
        if len(doc.getElementsByTagName('start_restart')) == 0:
            self.conf_xml.definir_champ(champ_restart)
 
        self.conf_xml.definir_champ(champ_restart,
                                    attribut = 'path',
                                    valeur_attribut = ch_restart)
        return 0



