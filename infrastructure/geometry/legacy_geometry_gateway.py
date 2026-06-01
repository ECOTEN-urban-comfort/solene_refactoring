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

from shutil import copy2
from typing import Any

from application.ports.geometry_gateway import GeometryGateway
from domain.geometry import LegacyExtractedGeometry, LegacySoleneGeometry, PreparedGeometryInputs
from domain.simulation_definition import SimulationBootstrap
from domain.simulation_state import SimulationState
from infrastructure.geometry.legacy.hdfFile import MedFile, CplFile
from infrastructure.geometry.legacy.famille import importer_familles_xml


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
        bootstrap = self._require_bootstrap_definition(state)

        stage_dir = state.workspace / "temp" / "geometry_inputs"
        stage_dir.mkdir(parents=True, exist_ok=True)

        staged_med_file = stage_dir / bootstrap.input_files.med_file.name
        staged_famille_file = stage_dir / bootstrap.input_files.famille_file.name
        staged_materiau_file = stage_dir / bootstrap.input_files.materiau_file.name

        # Copy into a deterministic technical area so later adapters do not depend
        # directly on the original case-input folder.
        copy2(bootstrap.input_files.med_file, staged_med_file)
        copy2(bootstrap.input_files.famille_file, staged_famille_file)
        copy2(bootstrap.input_files.materiau_file, staged_materiau_file)

        # Canonical sauvegarde targets inherited from the legacy workflow.
        sauvegarde_geom_med = state.workspace / "sauvegarde" / "geom_med.cpl"
        sauvegarde_geom_sol = state.workspace / "sauvegarde" / "geom_sol.cpl"

        return PreparedGeometryInputs(
            stage_dir=stage_dir,
            staged_med_file=staged_med_file,
            staged_famille_file=staged_famille_file,
            staged_materiau_file=staged_materiau_file,
            sauvegarde_geom_med=sauvegarde_geom_med,
            sauvegarde_geom_sol=sauvegarde_geom_sol,
            initialization_mode="med",
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

    def extract_geometry(
        self,
        state: SimulationState,
    ) -> LegacyExtractedGeometry:
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
        familles = importer_familles_xml(str(prepared.staged_famille_file))

        # Step 3: enrich family library with materials.
        familles.importer_materiaux_from_xml(str(prepared.staged_materiau_file))

        # Step 4: bind family numbers extracted from MED.
        familles.attribuer_num_familles(geom_med.familles)

        return LegacyExtractedGeometry(
            prepared_inputs=prepared,
            geom_med=geom_med,
            familles=familles,
            n_points=getattr(geom_med, "n_points", 0),
            n_triangles=getattr(geom_med, "n_triangles", 0),
            n_tetras=getattr(geom_med, "n_tetras", 0),
        )
    
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
        extracted = self._require_extracted_geometry(state)
        prepared = extracted.prepared_inputs

        if prepared.sauvegarde_geom_sol.is_file():
            geom_cpl = CplFile(str(prepared.sauvegarde_geom_sol))
            geom_cpl.charger_geom()
            geom_sol = geom_cpl.geom
            geom_sol.nom = "geom_sol"

            return LegacySoleneGeometry(
                extracted_geometry=extracted,
                geom_med=extracted.geom_med,
                geom_sol=geom_sol,
                geom_sol_masque=None,
                from_cache=True,
                n_sol_triangles=getattr(geom_sol, "n_triangles", 0),
            )

        # Fresh Solene-side branch
        geom_med = extracted.geom_med
        familles = extracted.familles

        # Equivalent of self.geom_med.reconstruire_geom()
        geom_med.reconstruire_geom()

        # Equivalent of self.extraire_geom_solene()
        liste_num = []
        liste_num_masque = []

        for fam_key in list(familles.familles.keys()):
            famille = familles.familles[fam_key]
            if famille.classe in familles.CARAC_CLASSE:
                carac = familles.CARAC_CLASSE[famille.classe]
            else:
                carac = None

            if carac and carac["solene"]:
                liste_num_masque.append(famille.num)
                if carac["emissivite"]:
                    liste_num.append(famille.num)

        geom_sol = geom_med.creer_sous_geom(
            nom="geom_sol",
            liste_famille=liste_num,
        )
        geom_sol_masque = geom_med.creer_sous_geom(
            nom="geom_sol_masque",
            liste_famille=liste_num_masque,
        )

        # Equivalent of enregistrer_sim() for the geometry cache side only.
        geom_sol_cpl = CplFile(str(prepared.sauvegarde_geom_sol), geom_sol)
        geom_sol_cpl.enregistrer_geom()

        geom_med_cpl = CplFile(str(prepared.sauvegarde_geom_med), geom_med)
        geom_med_cpl.enregistrer_geom()

        return LegacySoleneGeometry(
            extracted_geometry=extracted,
            geom_med=geom_med,
            geom_sol=geom_sol,
            geom_sol_masque=geom_sol_masque,
            from_cache=False,
            n_sol_triangles=getattr(geom_sol, "n_triangles", 0),
        )

    def _require_bootstrap_definition(
        self,
        state: SimulationState,
    ) -> SimulationBootstrap:
        """
        Temporary guard while SimulationState.definition is still broadly typed.
        """
        if not isinstance(state.definition, SimulationBootstrap):
            raise TypeError(
                "SimulationState.definition must be SimulationBootstrap "
                "before geometry gateway operations can run."
            )
        return state.definition

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