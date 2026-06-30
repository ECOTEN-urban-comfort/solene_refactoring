from __future__ import annotations

from pathlib import Path
from typing import Any

import numpy as np

from infrastructure.solene.sol_file import read_val, write_val
from infrastructure.solene.utils import ecrire_fichier


class DatValBridge:
    """
    Bridge between Saturne DAT files and Solene VAL files.

    Legacy compatibility:
    - single-process DAT->VAL / VAL->DAT mappings
    - multiprocess DAT->VAL / VAL->DAT mappings
    """

    def __init__(self, geom_sol: Any) -> None:
        self.geom_sol = geom_sol

        # multiprocess mappings
        self.dic_val2dat: dict[int, list[tuple[int, int]]] = {}
        self.dic_dat2val: dict[int, dict[int, int]] = {}

        # single-process mappings
        self.lien_dat_val: dict[int, int] = {}
        self.lien_val_dat: dict[int, int] = {}

    def connect_single(self, fichier_dat: str) -> None:
        """
        Build DAT<->VAL connectivity for a single-process Saturne DAT file.

        Parameters
        ----------
        fichier_dat:
            DAT filename containing rows in legacy format, typically:
                x y z value
        """
        dic_lien, dic_inv_lien = self._recuperer_xyz_val(fichier_dat)

        self.lien_dat_val = dic_lien
        self.lien_val_dat = dic_inv_lien

    def connect_multipro(self, fichier_dat: str, n_proc: int) -> None:
        """
        Build DAT<->VAL connectivity for multiprocess Saturne outputs.

        Parameters
        ----------
        fichier_dat:
            Base DAT filename. Can be passed with or without `.dat`.
            The actual files are expected as:
                <base>_0, <base>_1, ..., <base>_<n_proc-1>
        n_proc:
            Number of Saturne partitions / DAT files.
        """
        base = self._strip_dat_suffix(fichier_dat)

        self.dic_val2dat = {j: [] for j in range(self.geom_sol.n_triangles)}
        self.dic_dat2val = {}

        for proc_idx in range(n_proc):
            nom_fichier = f"{base}_{proc_idx}"
            dic_lien, dic_inv_lien = self._recuperer_xyz_val(nom_fichier)

            proc_id = proc_idx + 1
            self.dic_dat2val[proc_id] = dic_lien

            for no_val, no_dat in dic_inv_lien.items():
                self.dic_val2dat[no_val].append((proc_id, no_dat))

    def dat2val(
        self,
        fichier_dat: str,
        fichier_val: str,
        *,
        n_proc: int = 1,
        return_val: bool = False,
    ):
        """
        Convert DAT file(s) into a Solene VAL file.
        """
        if n_proc == 1:
            valeurs = self._dat2val_single(fichier_dat)
        else:
            valeurs = self._dat2val_multipro(fichier_dat, n_proc)

        write_val(
            name=fichier_val,
            geom=self.geom_sol,
            data=valeurs,
        )

        if return_val:
            return valeurs

    def val2dat(
        self,
        fichier_val: str,
        fichier_dat: str,
        *,
        n_proc: int = 1,
        xyz_cdg: bool = False,
    ) -> None:
        """
        Convert a Solene VAL file into DAT file(s).
        """
        if n_proc == 1:
            self._val2dat_single(fichier_val, fichier_dat, xyz_cdg=xyz_cdg)
        else:
            self._val2dat_multipro(
                fichier_val,
                fichier_dat,
                n_proc=n_proc,
                xyz_cdg=xyz_cdg,
            )

    def _dat2val_single(self, fichier_dat: str) -> np.ndarray:
        if not self.lien_dat_val:
            raise ValueError(
                "Single-process DAT->VAL conversion requires connect_single() first."
            )

        lignes = Path(fichier_dat).read_text(encoding="utf-8").splitlines()
        val_valeurs = np.zeros(self.geom_sol.n_triangles, dtype=float)

        for i, ligne in enumerate(lignes):
            if not ligne.strip():
                continue
            no_val = self.lien_dat_val[i]
            val_valeurs[no_val] = self._extract_dat_value(ligne)

        return val_valeurs

    def _dat2val_multipro(self, fichier_dat: str, n_proc: int) -> np.ndarray:
        if not self.dic_dat2val:
            raise ValueError(
                "Multiprocess DAT->VAL conversion requires connect_multipro() first."
            )

        base = self._strip_dat_suffix(fichier_dat)
        val_valeurs = np.zeros(self.geom_sol.n_triangles, dtype=float)

        for proc_idx in range(n_proc):
            proc_id = proc_idx + 1
            mapping = self.dic_dat2val.get(proc_id)
            if mapping is None:
                raise ValueError(
                    f"Missing DAT->VAL mapping for processor {proc_id}. "
                    "Did you run connect_multipro() with the same n_proc?"
                )

            lignes = Path(f"{base}_{proc_idx}").read_text(encoding="utf-8").splitlines()

            for no_dat, ligne in enumerate(lignes):
                if not ligne.strip():
                    continue
                no_val = mapping[no_dat]
                val_valeurs[no_val] = self._extract_dat_value(ligne)

        return val_valeurs

    def _val2dat_single(self, fichier_val: str, fichier_dat: str, *, xyz_cdg: bool) -> None:
        if not self.lien_val_dat:
            raise ValueError(
                "Single-process VAL->DAT conversion requires connect_single() first."
            )

        val_valeurs = read_val(name=fichier_val, geom=self.geom_sol)
        dat_lignes = [""] * len(val_valeurs)
        cdg = self.geom_sol.triangles.cdg

        for no_val, valeur in enumerate(val_valeurs):
            no_dat = self.lien_val_dat[no_val]
            parts: list[str] = []

            if xyz_cdg:
                for j in range(3):
                    parts.append(str(round(float(cdg[no_val][j]), 2)))

            parts.append(str(valeur))
            dat_lignes[no_dat] = "\t".join(parts)

        texte = "".join(f"{ligne}\n" for ligne in dat_lignes)
        ecrire_fichier(fichier_dat, texte)

    def _val2dat_multipro(
        self,
        fichier_val: str,
        fichier_dat: str,
        *,
        n_proc: int,
        xyz_cdg: bool,
    ) -> None:
        if not self.dic_val2dat:
            raise ValueError(
                "Multiprocess VAL->DAT conversion requires connect_multipro() first."
            )

        base = self._strip_dat_suffix(fichier_dat)
        val_valeurs = read_val(name=fichier_val, geom=self.geom_sol)
        cdg = self.geom_sol.triangles.cdg

        dic_dat: dict[int, dict[int, str]] = {i + 1: {} for i in range(n_proc)}

        for no_val, valeur in enumerate(val_valeurs):
            mappings = self.dic_val2dat.get(no_val, [])
            for proc_id, no_dat in mappings:
                parts: list[str] = []

                if xyz_cdg:
                    for j in range(3):
                        parts.append(str(round(float(cdg[no_val][j]), 2)))

                parts.append(str(valeur))
                dic_dat[proc_id][no_dat] = "\t".join(parts)

        for proc_idx in range(n_proc):
            proc_id = proc_idx + 1
            nom_fichier = f"{base}_{proc_idx}"

            lignes = []
            for no_dat in sorted(dic_dat[proc_id].keys()):
                lignes.append(dic_dat[proc_id][no_dat])

            ecrire_fichier(nom_fichier, "".join(f"{ligne}\n" for ligne in lignes))

    def _recuperer_xyz_val(self, nom_fichier: str) -> tuple[dict[int, int], dict[int, int]]:
        """
        Read one DAT file containing XYZ + value rows and build connectivity:
        - con:     DAT row index -> Solene triangle index
        - con_inv: Solene triangle index -> DAT row index
        """
        cdg = self.geom_sol.triangles.cdg.transpose()

        lignes = Path(nom_fichier).read_text(encoding="utf-8").splitlines()
        dat = []

        for ligne in lignes:
            if not ligne.strip():
                continue
            dat.append(ligne.split())

        dat_np = np.float32(np.array(dat))
        con: dict[int, int] = {}
        con_inv: dict[int, int] = {}

        for i in range(len(dat_np)):
            i_dat = dat_np[i]
            lien = self._trouver_cdg_xyz(i_dat, cdg)

            if lien is None:
                raise ValueError(
                    f"Could not map DAT row {i} from '{nom_fichier}' "
                    f"to a unique Solene triangle CDG."
                )

            con[i] = lien
            con_inv[lien] = i

        return con, con_inv

    @staticmethod
    def _trouver_cdg_xyz(i_dat: np.ndarray, liste_cdg: np.ndarray) -> int | None:
        epsilon = 0.05

        liste_0 = np.arange(len(liste_cdg[0]))
        liste_x = liste_0[np.abs(liste_cdg[0] - i_dat[0]) < epsilon]
        liste_y = liste_x[np.abs(liste_cdg[1][liste_x] - i_dat[1]) < epsilon]
        liste_z = liste_y[np.abs(liste_cdg[2][liste_y] - i_dat[2]) < epsilon]

        if len(liste_z) == 1:
            return int(liste_z[0])

        if len(liste_z) == 0:
            return None

        return None

    @staticmethod
    def _strip_dat_suffix(path: str) -> str:
        return path[:-4] if path.endswith(".dat") else path

    @staticmethod
    def _extract_dat_value(ligne: str) -> float:
        tokens = ligne.split()
        if not tokens:
            raise ValueError("Encountered empty DAT line while extracting value.")

        if len(tokens) >= 4:
            return float(tokens[3])

        return float(tokens[-1])