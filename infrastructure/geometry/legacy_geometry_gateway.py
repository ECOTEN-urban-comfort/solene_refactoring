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

from application.ports.geometry_gateway import GeometryGateway
from domain.geometry import LegacyExtractedGeometry, PreparedGeometryInputs
from domain.simulation_definition import SimulationBootstrap
from domain.simulation_state import SimulationState
from infrastructure.geometry.legacy.hdfFile import MedFile
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

    def extract(
        self,
        state: SimulationState,
    ) -> LegacyExtractedGeometry:
        """
        Execute the first true legacy geometry extraction step.

        What this method transfers from legacy `initialiser_med(...)`:
        --------------------------------------------------------------
        1. open MED geometry through MedFile(...)
        2. call extraire_geom()
        3. load families from famille.xml
        4. load materials from materiau.xml
        5. assign MED family numbers to loaded families

        What it intentionally does NOT do yet:
        --------------------------------------
        - charger_cpl('sat')
        - reconstruire_geom()
        - extraire_geom_solene()
        - exporter_geom_solene()
        - enregistrer_sim()
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