# Unified infrastructure adapter for the current legacy geometry workflow.
#
# This module implements the geometry gateway by combining two technical tasks:
# - preparing staged working copies of geometry-related input files,
# - executing the first legacy MED/family/material extraction step.
#
# More specifically, it:
# - creates a deterministic geometry staging area,
# - copies MED, famille.xml, and materiau.xml into it,
# - invokes legacy MED geometry extraction,
# - loads family definitions from XML,
# - enriches them with material data,
# - binds MED family numbers to the loaded family library,
# - returns typed wrappers around the prepared and extracted geometry results.
#
# This module is the current technical bridge between the refactored codebase
# and the preserved legacy geometry implementation.

from shutil import copyfile
from typing import Any
from pathlib import Path

from application.ports.geometry_gateway import GeometryGateway
from domain.geometry import PreparedGeometryInputs, SoleneGeometryArtifacts
from domain.simulation_definition import SimulationBootstrap
from domain.simulation_state import SimulationState
from infrastructure.solene.hdfFile import MedFile, CplFile
from infrastructure.solene.sol_file import write_cir
from infrastructure.solene.famille import importer_familles_xml, Familles


class LegacyGeometryGateway(GeometryGateway):
    """
    Unified technical geometry gateway.

    Why this merged adapter exists:
    -------------------------------
    At the current refactoring stage, geometry input staging and first legacy
    MED/family/material extraction are still part of one coherent technical flow.

    So instead of having:
        - FilesystemGeometryPreparationGateway
        - LegacyMedGeometryExtractionGateway

    we keep one infrastructure adapter that offers two methods:
        - prepare_inputs(...)
        - extract_legacy_geometry(...)

    This reduces file count while preserving a meaningful technical boundary.
    """

    def prepare_inputs(self, state: SimulationState) -> PreparedGeometryInputs:
        """
        Prepare a deterministic technical staging area for geometry-related inputs.

        What this method does:
        ----------------------
        - creates a staging directory in the runtime workspace
        - copies MED / famille.xml / materiau.xml there
        - exposes the canonical sauvegarde targets used later by legacy geometry flow

        What it intentionally does NOT do yet:
        --------------------------------------
        - parse MED
        - read XML content
        - reconstruct geometry
        - export Solene geometry
        """
        bootstrap = state.require_bootstrap_definition()

        stage_dir = state.workspace / "temp" / "geometry_inputs"
        stage_dir.mkdir(parents=True, exist_ok=True)

        staged_med_file = stage_dir / bootstrap.input_files.med_file.name
        staged_famille_file = stage_dir / bootstrap.input_files.famille_file.name
        staged_materiau_file = stage_dir / bootstrap.input_files.materiau_file.name

        # Copy into a deterministic technical area so later adapters do not depend
        # directly on the original case-input folder.
        copyfile(bootstrap.input_files.med_file, staged_med_file)
        copyfile(bootstrap.input_files.famille_file, staged_famille_file)
        copyfile(bootstrap.input_files.materiau_file, staged_materiau_file)

        # Canonical sauvegarde targets inherited from the legacy workflow.
        sauvegarde_dir = state.workspace / "sauvegarde"
        simul_sol_dir = state.workspace / "simulSol"

        return PreparedGeometryInputs(
            staged_med_file=staged_med_file,
            staged_famille_file=staged_famille_file,
            staged_materiau_file=staged_materiau_file,
            sauvegarde_dir=sauvegarde_dir,
            simul_sol_dir=simul_sol_dir
        )

    def extract_families(
        self,
        bootstrap: SimulationBootstrap,
        prepared: PreparedGeometryInputs,
    ) -> Familles:
        """
        Unified technical geometry gateway.

        This adapter currently owns:
            - deterministic staging of geometry-related inputs,
            - legacy `.cpl` MED geometry cache access,
            - first legacy MED/family/material extraction.
        """

        # Step 1: extract MED geometry using the old reader.
        med_file = MedFile(str(prepared.staged_med_file))
        geom_med = med_file.extraire_geom()

        # Step 2: load family definitions from XML.
        surface_model = bootstrap.settings.surface_model
        familles = importer_familles_xml(str(prepared.staged_famille_file), surface_model)

        # Step 3: enrich family library with materials.
        familles.importer_materiaux_from_xml(str(prepared.staged_materiau_file))

        # Step 4: bind family numbers extracted from MED.
        familles.attribuer_num_familles(geom_med.familles)

        return familles
    
    def build_solene_geometry(
        self,
        prepared: PreparedGeometryInputs,
        families: Familles,
    ) -> SoleneGeometryArtifacts:
        """
        Refactored equivalent of the Solene-side geometry branch in
        `SimulationCouplee.initialiser_med(...)`.

        Current scope:
            - load geom_sol.cpl if it already exists,
            - otherwise reconstruct geom_med,
            - derive geom_sol and geom_sol_masque,
            - save geom_sol.cpl and updated geom_med.cpl.

        Intentionally not included yet:
            - exporter_geom_solene() / .cir export
            - SolCommand path handling
            - SolEnv creation
        """
        carac_classe = families.carac_classe

        if self._has_solene_geometry_artifacts(prepared):
            return SoleneGeometryArtifacts(
                geom_med_cpl=prepared.geom_med_cpl,
                geom_sol_cpl=prepared.geom_sol_cpl,
                scene_cir=prepared.scene_cir,
                masque_cir=prepared.masque_cir,
                n_sol_triangles=self._read_n_triangles(
                    prepared.geom_sol_cpl
                ),
            )

        # Fresh Solene-side branch
        med_file = MedFile(str(prepared.staged_med_file))
        geom_med = med_file.extraire_geom()
        
        # Equivalent of self.geom_med.reconstruire_geom()
        geom_med.reconstruire_geom()

        liste_num, liste_num_masque = self._collect_solene_family_lists(
            familles=families,
            carac_classe=carac_classe,
        )

        geom_sol = geom_med.creer_sous_geom(
            nom="geom_sol",
            liste_famille=liste_num,
        )
        n_sol_triangles = int(geom_sol.n_triangles)
        
        geom_sol_masque = geom_med.creer_sous_geom(
            nom="geom_sol_masque",
            liste_famille=liste_num_masque,
        )

        self._save_geom(prepared.geom_med_cpl, geom_med)
        self._save_geom(prepared.geom_sol_cpl, geom_sol)

        prepared.scene_cir.parent.mkdir(parents=True, exist_ok=True)
        prepared.masque_cir.parent.mkdir(parents=True, exist_ok=True)

        write_cir(
            name=str(prepared.scene_cir.with_suffix("")),
            geom=geom_sol,
            faces=False,
        )
        write_cir(
            name=str(prepared.masque_cir.with_suffix("")),
            geom=geom_sol_masque,
            faces=True,
        )

        return SoleneGeometryArtifacts(
            geom_med_cpl=prepared.geom_med_cpl,
            geom_sol_cpl=prepared.geom_sol_cpl,
            scene_cir=prepared.scene_cir,
            masque_cir=prepared.masque_cir,
            n_sol_triangles=n_sol_triangles,
        )
    
    @staticmethod
    def _has_solene_geometry_artifacts(prepared: PreparedGeometryInputs) -> bool:
        return (
            prepared.geom_med_cpl.is_file()
            and prepared.geom_sol_cpl.is_file()
            and prepared.scene_cir.is_file()
            and prepared.masque_cir.is_file()
        )

    @staticmethod
    def _save_geom(path: Path, geom) -> None:
        geom_cpl = CplFile(str(path), geom)
        geom_cpl.enregistrer_geom()

    @staticmethod
    def _collect_solene_family_lists(familles, carac_classe) -> tuple[list[int], list[int]]:
        liste_num = []
        liste_num_masque = []

        for famille in familles.familles.values():
            carac = carac_classe.get(famille.classe)
            if not carac or not carac["solene"]:
                continue

            liste_num_masque.append(famille.num)
            if carac["emissivite"]:
                liste_num.append(famille.num)

        return liste_num, liste_num_masque
    
    @staticmethod
    def _read_n_triangles(geom_cpl_path: Path) -> int:
        cpl_file = CplFile(str(geom_cpl_path))
        cpl_file.charger_geom()

        geom = cpl_file.geom
        if geom is None:
            raise ValueError(
                f"Geometry could not be loaded from {geom_cpl_path}"
            )

        n_triangles = getattr(geom, "n_triangles", None)

        if n_triangles is None:
            triangles = getattr(geom, "triangles", None)
            triangle_points = getattr(triangles, "points", None)

            if triangle_points is None:
                raise ValueError(
                    "Loaded geom_sol does not contain n_triangles "
                    f"or triangles.points: {geom_cpl_path}"
                )

            n_triangles = len(triangle_points)

        return int(n_triangles)