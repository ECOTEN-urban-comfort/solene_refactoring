from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import os
from math import pi, sqrt, sin

from infrastructure.solene.timeStep import convertir_ts_sol_liste, ecrire_ts_sol


SUPPORTED_METEO_TYPES = {"RT", "ONEVU", "HEPIA", "Khaled", "ILYES"}


@dataclass(frozen=True)
class MeteoSample:
    """
    One parsed meteo sample in a normalized internal representation.
    """
    T: float
    v: float
    w: float | None = None
    direction: float | None = None

    # Optional extra fields preserved from source files when available.
    raw: dict[str, Any] | None = None

    def to_legacy_dict(self) -> dict[str, Any]:
        """
        Convert back to the dictionary shape expected by legacy Solene code.
        """
        result = {
            "T": self.T,
            "v": self.v,
        }
        if self.w is not None:
            result["w"] = self.w
        if self.direction is not None:
            result["direction"] = self.direction
        if self.raw:
            for key, value in self.raw.items():
                if key not in result:
                    result[key] = value
        return result

def parser_fichier_meteo_RT(chemin):
    """
    recupere sous forme d'une liste de dictionnaires les parametres meteo
    depuis un fichier meteo au format RT
    """
    meteo = []
    fichier = open(chemin, 'r')
    fichier.readline()
    lignes = fichier.readlines()
    for heure in range(len(lignes)):
        meteo_h = {}
        ligne = lignes[heure].replace('"', '')
        ligne = ligne.replace(',', '.')
        ligne = ligne.split()
        meteo_h['T'] = round(float(ligne[12]), 3)
        meteo_h['w'] = round(float(ligne[13]), 3)
        meteo_h['Tciel'] = round(float(ligne[16]), 3)
        meteo_h['v'] = round(float(ligne[17]), 3)
        meteo.append(meteo_h)

    return meteo


def parser_fichier_meteo_simple(chemin_fichier, jour=None, mois=None):
    """
    parse les fichier meteo au format 'hh mm valeur'
    """
    nom_fichier = os.path.split(chemin_fichier)[1]

    if not jour:
        if nom_fichier[-4:] == '.txt':
            nom_fichier = nom_fichier[:-4]

        jour = int(nom_fichier.split('_')[-2])
        mois = int(nom_fichier.split('_')[-1])

    fichier = open(chemin_fichier)
    fichier = fichier.readlines()

    dic = {}
    for ligne in fichier:
        ligne = ligne.split()
        heure = ligne[0]
        minute = ligne[1]
        ts_s = convertir_ts_sol_liste(jour, mois, heure, minute)
        suffixe = ecrire_ts_sol(ts_s)
        dic[suffixe] = ligne[2]

    return dic


def parser_fichier_meteo_khaled(chemin_fichier):
    """
    importe la meteo depuis les fichiers de Khaled (?)
    """
    fichier = open(chemin_fichier)
    fichier = fichier.readlines()

    dic = {}

    for ligne in fichier[1:]:
        ligne = ligne.split()
        jour = int(ligne[0].split('/')[0])
        mois = int(ligne[0].split('/')[1])
        heure = int(ligne[1].split(':')[0])
        minutes = int(ligne[1].split(':')[1])
        IR_ciel = float(ligne[2])
        Tair = float(ligne[3])
        vit = float(ligne[4])
        z_0 = float(ligne[5])
        Tair = float(ligne[3])
        vit = float(ligne[4])
        z_0 = float(ligne[5])
        direction = float(ligne[6])
        ts_s = convertir_ts_sol_liste(jour, mois, heure, minutes)
        suffixe = ecrire_ts_sol(ts_s)
        dic[suffixe] = {
            'v': vit,
            'z0': z_0,
            'direction': direction,
            'T': Tair,
            'flux_ir': IR_ciel,
        }

    return dic


def parser_fichier_meteo_ONEVU(chemin_fichier):
    """
    importe la meteo depuis les fichiers fournis par Jean Michel
    """
    meteo = []
    fichier = open(chemin_fichier, 'r')
    fichier.readline()
    lignes = fichier.readlines()
    for heure in range(len(lignes)):
        meteo_h = {}
        ligne = lignes[heure].replace('"', '')
        ligne = ligne.split()
        meteo_h['v'] = round(float(ligne[2]), 3)
        meteo_h['direction'] = round(float(ligne[3]), 3)
        meteo_h['Global'] = round(float(ligne[4]), 3)
        meteo_h['flux_ir'] = round(float(ligne[5]))
        meteo_h['T'] = round(float(ligne[6]), 3)
        meteo_h['HR'] = round(float(ligne[7]), 3)
        meteo_h['w'] = round(float(ligne[9]), 3)
        if meteo_h['flux_ir'] == -999:
            meteo_h['flux_ir'] = 5.5 * meteo_h['T'] + 213

        meteo.append(meteo_h)

    return meteo


def parser_fichier_meteo_HEPIA(chemin):
    """
    recupere sous forme d'une liste de dictionnaires les parametres meteo
    depuis un fichier meteo au format RT
    """
    meteo = []
    fichier = open(chemin, 'r')
    entete = fichier.readline()
    entete = entete.split()
    lignes = fichier.readlines()
    lignes = lignes[4:]
    for heure in range(len(lignes)):
        meteo_h = {}
        ligne = lignes[heure].replace(',', '.')
        ligne = ligne.split(';')
        meteo_h['Irrig_Tot(1)'] = round(float(ligne[6]), 2)
        meteo_h['Irrig_Tot(2)'] = round(float(ligne[7]), 2)
        meteo_h['Irrig_Tot(3)'] = round(float(ligne[8]), 2)
        meteo_h['Drain_Tot(1)'] = round(float(ligne[9]), 2)
        meteo_h['Drain_Tot(2)'] = round(float(ligne[10]), 2)
        meteo_h['Drain_Tot(3)'] = round(float(ligne[11]), 2)
        meteo_h['Drain_Tot(4)'] = round(float(ligne[12]), 2)
        meteo_h['Load_Avg'] = round(float(ligne[13]), 2)
        meteo_h['T'] = round(float(ligne[14]), 2)
        meteo_h['HR'] = round(float(ligne[19]), 2)
        meteo_h['w'] = HR_to_HS(meteo_h['T'], meteo_h['HR'])
        meteo_h['v'] = round(float(ligne[21]), 2)
        meteo_h['Gh'] = round(float(ligne[16]), 2)
        meteo_h['Dh'] = round(float(ligne[17]), 2)
        meteo_h['Gv'] = round(float(ligne[18]), 2)
        meteo_h['T_IR1'] = round(float(ligne[40]), 2)
        meteo_h['T_IR2'] = round(float(ligne[41]), 2)
        meteo_h['T_IR3'] = round(float(ligne[42]), 2)
        meteo_h['T_IR4'] = round(float(ligne[43]), 2)
        meteo_h['TC1_1'] = round(float(ligne[44]), 2)
        meteo_h['TC1_2'] = round(float(ligne[45]), 2)
        meteo_h['TC1_3'] = round(float(ligne[46]), 2)
        meteo_h['TC1_4'] = round(float(ligne[47]), 2)
        meteo_h['Teau'] = round(float(ligne[48]), 2)
        meteo_h['flux_ir'] = 5.5 * meteo_h['T'] + 213

        try:
            meteo_h['direction'] = round(float(ligne[20]), 2)
        except:
            meteo_h['direction'] = 0

        meteo.append(meteo_h)

    return meteo


def parser_fichier_meteo_ILYES(chemin):
    meteo = []
    fichier = open(chemin, 'r')
    ligne = fichier.readline()
    while 'mm/dd/yyyy' not in ligne:
        ligne = fichier.readline()
    ligne = fichier.readline()
    while ligne:
        meteo_h = {}
        ligne = ligne.split()
        meteo_h['T'] = round(float(ligne[17]), 2)
        meteo_h['Global'] = round(float(ligne[11]), 3)
        meteo_h['Diffus'] = round(float(ligne[12]), 2)
        meteo_h['direction'] = round(float(ligne[15]), 3)
        meteo_h['v'] = round(float(ligne[16]), 2)
        meteo_h['HR'] = round(float(ligne[14]), 2)
        meteo_h['w'] = HR_to_HS(meteo_h['T'], meteo_h['HR'])
        meteo_h['flux_ir'] = 5.5 * meteo_h['T'] + 213
        meteo.append(meteo_h)
        ligne = fichier.readline()
    return meteo


def HR_to_HS(Tair, HR):
    """
    converti l'humidite relative en humidite specifique
    """
    T_radian = Tair * pi / 180.0
    p_sat = 610.7 * (1 + sqrt(2) * sin(T_radian / 3)) ** 8.827
    HS = (0.622 * p_sat * HR) / (101325 - (0.378 * p_sat * HR))
    return HS

def parse_meteo_file(fichier_meteo, type_meteo):
    if type_meteo == "RT":
        return parser_fichier_meteo_RT(fichier_meteo)
    if type_meteo == "ONEVU":
        return parser_fichier_meteo_ONEVU(fichier_meteo)
    if type_meteo == "HEPIA":
        return parser_fichier_meteo_HEPIA(fichier_meteo)
    if type_meteo == "Khaled":
        return parser_fichier_meteo_khaled(fichier_meteo)
    if type_meteo == "ILYES":
        return parser_fichier_meteo_ILYES(fichier_meteo)
    raise ValueError(f"Unsupported meteo file type: {type_meteo}")