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
from domain.geometry import LegacyExtractedGeometry, LegacySoleneGeometry, PreparedGeometryInputs, SoleneGeometryArtifacts
from domain.simulation_state import SimulationState
from domain.artifact_keys import FAMILLES
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
    
    def has_saved_med_geometry(self, state: SimulationState) -> bool:
        prepared = self._require_prepared_inputs(state)
        return prepared.sauvegarde_geom_med.is_file()

    def load_saved_med_geometry(self, state: SimulationState) -> Any:
        prepared = self._require_prepared_inputs(state)

        geom_cpl = CplFile(str(prepared.sauvegarde_geom_med))
        geom_cpl.charger_geom()

        geom_med = geom_cpl.geom
        geom_med.nom = "geom_med"
        return geom_med

    def save_med_geometry(self, state: SimulationState, geom_med: Any) -> None:
        prepared = self._require_prepared_inputs(state)

        geom_cpl = CplFile(str(prepared.sauvegarde_geom_med), geom_med)
        geom_cpl.enregistrer_geom()

    def extract_familles(
        self,
        state: SimulationState,
    ) -> Familles:
        """
        Unified technical geometry gateway.

        This adapter currently owns:
            - deterministic staging of geometry-related inputs,
            - legacy `.cpl` MED geometry cache access,
            - first legacy MED/family/material extraction.
        """
        prepared = self._require_prepared_inputs(state)

        # Step 1: extract MED geometry using the old reader.
        med_file = MedFile(str(prepared.staged_med_file))
        geom_med = med_file.extraire_geom()

        # Step 2: load family definitions from XML.
        bootstrap = state.require_bootstrap_definition()
        surface_model = bootstrap.settings.surface_model
        familles = importer_familles_xml(str(prepared.staged_famille_file), surface_model)

        # Step 3: enrich family library with materials.
        familles.importer_materiaux_from_xml(str(prepared.staged_materiau_file))

        # Step 4: bind family numbers extracted from MED.
        familles.attribuer_num_familles(geom_med.familles)

        return familles
    
    def build_solene_geometry(
        self,
        state: SimulationState,
    ) -> LegacySoleneGeometry:
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
        prepared = self._require_prepared_inputs(state)
        familles = state.results[FAMILLES]
        carac_classe = familles.carac_classe

        if self._has_solene_geometry_artifacts(prepared):
            return SoleneGeometryArtifacts(
                geom_med_cpl=prepared.sauvegarde_geom_med,
                geom_sol_cpl=prepared.sauvegarde_geom_sol,
                scene_cir=prepared.scene_cir,
                masque_cir=prepared.masque_cir,
            )

        # Fresh Solene-side branch
        med_file = MedFile(str(prepared.staged_med_file))
        geom_med = med_file.extraire_geom()
        
        # Equivalent of self.geom_med.reconstruire_geom()
        geom_med.reconstruire_geom()

        liste_num, liste_num_masque = self._collect_solene_family_lists(
            familles=familles,
            carac_classe=carac_classe,
        )

        geom_sol = geom_med.creer_sous_geom(
            nom="geom_sol",
            liste_famille=liste_num,
        )
        geom_sol_masque = geom_med.creer_sous_geom(
            nom="geom_sol_masque",
            liste_famille=liste_num_masque,
        )

        self._save_geom(prepared.sauvegarde_geom_med, geom_med)
        self._save_geom(prepared.sauvegarde_geom_sol, geom_sol)

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
            geom_med_cpl=prepared.sauvegarde_geom_med,
            geom_sol_cpl=prepared.sauvegarde_geom_sol,
            scene_cir=prepared.scene_cir,
            masque_cir=prepared.masque_cir,
        )
    
    @staticmethod
    def _has_solene_geometry_artifacts(prepared: PreparedGeometryInputs) -> bool:
        return (
            prepared.sauvegarde_geom_med.is_file()
            and prepared.sauvegarde_geom_sol.is_file()
            and prepared.scene_cir.is_file()
            and prepared.masque_cir.is_file()
        )

    @staticmethod
    def _save_geom(path: Path, geom) -> None:
        geom_cpl = CplFile(str(path), geom)
        geom_cpl.enregistrer_geom()

    @staticmethod
    def _load_geom(path: Path, nom: str):
        geom_cpl = CplFile(str(path))
        geom_cpl.charger_geom()
        geom = geom_cpl.geom
        geom.nom = nom
        return geom

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

    def _require_prepared_inputs(self, state: SimulationState) -> PreparedGeometryInputs:
        """
        Ensure geometry preparation already ran before extraction starts.
        """
        prepared = state.results.get("prepared_geometry_inputs")
        if prepared is None:
            raise ValueError(
                "Prepared geometry inputs are missing; run geometry preparation first."
            )
        return prepared
    
    def _require_extracted_geometry(self, state: SimulationState) -> LegacyExtractedGeometry:
        extracted = state.results.get("legacy_extracted_geometry")
        if extracted is None:
            raise ValueError(
                "Legacy extracted geometry is missing; run MED extraction first."
            )
        return extracted