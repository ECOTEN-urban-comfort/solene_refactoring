# -*- coding: utf-8 -*-
"""@package timeStep
Classe TimeStep

Created on Wed Jul 28 10:08:35 2010

@author :
    Laurent Malys, Laboratoire CERMA, UMR 1563
    laurent.malys@cerma.archi.fr
    
"""


import time
import os

def convertir_ts_sol_to_s(suffixe = '', fname = None):
    if fname:
        fname = fname[:-4].split('_')
        suffixe = '%s_%s_%s' (fname[-3], fname[-2], fname[-1])
        
    tps = time.strptime(suffixe, "%d_%m_%HH%M")
    seconde = (tps.tm_yday-1) *24*3600 + tps.tm_hour *3600 + tps.tm_min *60 + tps.tm_sec
    
    return seconde
    
def convertir_ts_sol_liste(jour, mois, heure, min):

    suffixe = '%s_%s_%sH%s' % (jour, mois, heure, min)
    
    seconde  = convertir_ts_sol_to_s(suffixe = suffixe)
    
    return seconde

def creer_h_sol(hh, mm):
    if len(str(hh)) <= 1:
        hh = str(0) + str(hh)
    if len(str(mm)) <= 1:
        mm = str(0) + str(mm)
        
    h_sol = '%s:%s' % (hh, mm)
    return h_sol
    
def ecrire_ts_sol(tps_en_sec = 0):
    sec_restantes = tps_en_sec

    tm_yday = int(tps_en_sec/(24*3600)) + 1
    sec_restantes -= (tm_yday-1) *24*3600
    
    tm_hour = int(sec_restantes/ 3600)
    sec_restantes -= tm_hour * 3600
    
    tm_min = int(sec_restantes/60)
    tm_sec = sec_restantes - tm_min * 60
    
    tps_string = '%s %s %s' % (tm_yday, tm_hour, tm_min)
    tps = time.strptime(tps_string, "%j %H %M")

    if len(str(tm_hour)) == 1:
        tm_hour = '0' + str(tm_hour)
    if len(str(tm_min)) == 1:
        tm_min = '0' + str(tm_min)
        
    tps_sol = '%s_%s_%sH%s' % (tps.tm_mday, tps.tm_mon, tm_hour, tm_min)
    
    return tps_sol

def creer_liste_ts(jour_d, mois_d, hh_d, mm_d,
                   jour_f, mois_f, hh_f, mm_f,
                   ts_hh, ts_mm):
    debut_s = convertir_ts_sol_liste(jour_d, mois_d, hh_d, mm_d)
    fin_s = convertir_ts_sol_liste(jour_f, mois_f, hh_f, mm_f)
    ts_s = ts_hh *3600 + ts_mm *60
    
    liste_ts = [debut_s]
    ts = debut_s
         
    if fin_s < debut_s:
        fin_year_s = convertir_ts_sol_to_s('31_12_23H00')
        while ts < fin_year_s:
            ts += ts_s
            liste_ts.append(ts)
        ts = 0
        liste_ts.append(0)
    while ts < fin_s:
        ts += ts_s
        liste_ts.append(ts)
         
        
    return liste_ts

def convertir_liste_ts_sol(liste_ts):
    liste_ts_sol=[]
    for ts in liste_ts:
        liste_ts_sol.append(ecrire_ts_sol(tps_en_sec = ts))
        
    return liste_ts_sol
    
def demander_interval():
    interval = []
    print('date de debut de simulation ?')
    debut = str(input('format jour mois heure minutes (JJ MM HH mm) : '))
    debut = debut.split()
    for elem in debut:
        try : 
            interval.append(int(elem))
        except:
            print(elem, 'n est pas un entier ?')
            elem = int(input('nouvelle valeur ?'))
            interval.append(elem)
            
    print('date de fin de simulation ?')
    fin = str(input('format jour mois heure minutes (JJ MM HH mm) : '))
    fin = fin.split()
    for elem in fin:
        try : 
            interval.append(int(elem))
        except:
            print(elem, 'n est pas un entier ?')
            elem = int(input('nouvelle valeur ?'))
            interval.append(elem)
            
    print('pas de temps ?')
    pas = str(input('format heure minutes (HH mm) : '))
    pas = pas.split()
    for elem in pas:
        try : 
            interval.append(int(elem))
        except:
            print(elem, 'n est pas un entier ?')
            elem = int(input('nouvelle valeur ?'))
            interval.append(elem)
            
    return interval
    
class TimeStep:
    def __init__(self):
        self.liste_ts = []
        self.liste_ts_sol = []
        self.liste_jours = []
        
        self.jour_d = 0
        self.mois_d = 0
        self.heure_d = 0
        self.min_d = 0
        self.jour_f = 0
        self.mois_f = 0
        self.heure_f = 0
        self.min_f = 0
        
        self.ts_hh = 0
        self.ts_jj = 0
        self.ts_s = 0
        
        self.n_ts = 0
        
    def definir_liste_ts(self, interval):
        self.jour_d = interval[0]
        self.mois_d = interval[1]
        self.heure_d = interval[2]
        self.min_d = interval[3]
        self.jour_f = interval[4]
        self.mois_f = interval[5]
        self.heure_f = interval[6]
        self.min_f = interval[7]
        self.ts_hh = interval[8]
        self.ts_mm = interval[9]
        self.ts_s = self.ts_hh *3600 + self.ts_mm *60
        self.ts_sol = creer_h_sol(self.ts_hh, self.ts_mm)
        
        
        self.liste_ts = creer_liste_ts(self.jour_d, self.mois_d, self.heure_d, self.min_d,
                                       self.jour_f, self.mois_f, self.heure_f, self.min_f,
                                       self.ts_hh, self.ts_mm)
        self.liste_ts_bis = creer_liste_ts(self.jour_d, self.mois_d, self.heure_d, self.min_d,
                                       self.jour_f, self.mois_f, self.heure_f, self.min_f,
                                       self.ts_hh, self.ts_mm)
                                           
        self.n_ts = len(self.liste_ts)
        self.liste_ts_sol = convertir_liste_ts_sol(self.liste_ts)
        self.convertir_liste_jours()
        
    def convertir_liste_jours(self):
        self.liste_jours = []
        liste_ts_sol = convertir_liste_ts_sol(self.liste_ts)
        compt_ts_sol = 0
        jour_debut_en_s = convertir_ts_sol_liste(self.jour_d, self.mois_d, 0, 0)

        if self.liste_ts[-1] < jour_debut_en_s:
            liste_jour_s = list(range(jour_debut_en_s, 3600*24*365, 3600*24))
            liste_jour_s += list(range(0, self.liste_ts[-1], 3600*24))
        else:
            liste_jour_s = list(range(jour_debut_en_s, self.liste_ts[-1], 3600*24))
        
        liste_ts_restant = self.liste_ts_bis
        
        ts = liste_ts_restant[0]
        
        for i in range(len(liste_jour_s)-1):
            jour_plus_un = liste_jour_s[i+1]
            jour_plus_un_nom = ecrire_ts_sol(liste_jour_s[i+1]).split('_')[0]
            
            jour_sol = liste_ts_sol[compt_ts_sol]
            jour = jour_sol.split('_')[0]
            mois = jour_sol.split('_')[1]
            heure = jour_sol.split('_')[2].replace('H',':')
            jour_new = jour_sol.split('_')[0]
            #while ts < jour_plus_un:
            while jour_new != jour_plus_un_nom:
                liste_ts_restant.remove(ts) 
                compt_ts_sol +=1
                ts = liste_ts_restant[0]
                jour_new = ecrire_ts_sol(ts).split('_')[0]
                
            heure_fin = liste_ts_sol[compt_ts_sol-1].split('_')[2].replace('H',':')
        
            self.liste_jours.append([jour, mois, heure, heure_fin, self.ts_sol])
        
        jour_sol = liste_ts_sol[compt_ts_sol]
        jour = jour_sol.split('_')[0]
        mois = jour_sol.split('_')[1]
        heure = jour_sol.split('_')[2].replace('H',':')            
        heure_fin = liste_ts_sol[-1].split('_')[2].replace('H',':')                                   
        self.liste_jours.append([jour, mois, heure, heure_fin, self.ts_sol]) 
        
    def jour(self, i):
        return time.localtime(self.liste_ts[i]-3600).tm_yday