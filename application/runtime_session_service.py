# Application service that turns a loaded simulation bootstrap into the first
# active runtime session.
#
# This module is responsible for:
# - creating the top-level runtime workspace directories for the case,
# - generating a run-specific identifier,
# - computing a configuration fingerprint for restart/consistency checks,
# - registering the known bootstrap input files as initial artifacts,
# - returning the first mutable SimulationState object.
#
# In the refactored architecture, this is the bridge between:
# - the static startup definition loaded from configuration, and
# - the dynamic state that will be updated by application services during the run.
#
# This module intentionally does not yet initialize solver-specific subtrees,
# geometry processing, coupling logic, or execution commands.

from ensurepip import bootstrap
from hashlib import sha256
import json
from pathlib import Path
from uuid import uuid4

from domain.simulation_definition import SimulationBootstrap
from domain.simulation_state import (
    ArtifactRef,
    SimulationPhase,
    SimulationState,
    StepStatus,
)
from domain.workspace import RuntimePaths
from domain.artifact_keys import (
    FAMILLE_XML,
    GEOMETRY_MED,
    MATERIAU_XML,
    METEO_CSV,
    SIM_SETTINGS_XML,
    INPUT_ARTIFACT,
)

class RuntimeSessionService:
    """
    Initialize the runtime session from already loaded bootstrap data.

    From `main.py` / `SimulationCouplee.__init__`:
        - creation of the case runtime folders
        - establishment of the working session rooted at the case directory

    What is intentionally NOT transferred here yet:
    -----------------------------------------------
        - Solene adapter / SolCommand instantiation
        - Saturne adapter / SatCommand instantiation
        - geometry initialization
        - meteo import
        - time-step preparation
        - interactive deletion / overwrite policy
    """

    def initialize(self, bootstrap: SimulationBootstrap) -> SimulationState:
        """
        Create the top-level runtime folders and return the first mutable
        SimulationState for the run.
        """
        state = SimulationState(
            run_id=self._build_run_id(),
            workspace=bootstrap.paths.case_root,
            phase=SimulationPhase.BOOTSTRAPPING,
            definition=bootstrap,
            artifacts=self._build_initial_artifacts(bootstrap),
        )

        state.set_step_status("bootstrapping", StepStatus.IN_PROGRESS)

        try:
            self._ensure_runtime_directories(bootstrap.paths)

        except Exception as exc:
            state.set_step_status("bootstrapping", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state

        state.set_step_status("bootstrapping", StepStatus.DONE)
        return state

    def _ensure_runtime_directories(self, paths: RuntimePaths) -> None:
        """
        Create only the top-level runtime directories.

        At this stage we only create the roots, not deeper solver-specific
        subtrees such as Solene `entree/sortie` or Saturne `MESH/DATA/RESU`.
        """
        for directory in (
            paths.case_root,
            paths.sauvegarde_dir,
            paths.temp_dir,
            paths.post_dir,
            paths.simul_sol_dir,
            paths.simul_sat_dir,
        ):
            directory.mkdir(parents=True, exist_ok=True)

    def _build_initial_artifacts(
        self,
        bootstrap: SimulationBootstrap,
    ) -> dict[str, ArtifactRef]:
        """
        Register the input artifacts that are already known to exist at the end of
        bootstrap loading.
        """
        return {
            GEOMETRY_MED: ArtifactRef(
                name=GEOMETRY_MED,
                path=bootstrap.input_files.med_file,
                artifact_type=INPUT_ARTIFACT,
            ),
            METEO_CSV: ArtifactRef(
                name=METEO_CSV,
                path=bootstrap.input_files.meteo_file,
                artifact_type=INPUT_ARTIFACT,
            ),
            SIM_SETTINGS_XML: ArtifactRef(
                name=SIM_SETTINGS_XML,
                path=bootstrap.input_files.sim_settings_file,
                artifact_type=INPUT_ARTIFACT,
            ),
            FAMILLE_XML: ArtifactRef(   
                name=FAMILLE_XML,
                path=bootstrap.input_files.famille_file,
                artifact_type=INPUT_ARTIFACT,
            ),
            MATERIAU_XML: ArtifactRef(
                name=MATERIAU_XML,
                path=bootstrap.input_files.materiau_file,
                artifact_type=INPUT_ARTIFACT,
            ),
        }

    def _build_run_id(self) -> str:
        """
        Generate a lightweight unique run identifier.

        This is new compared with the legacy script, where run identity is mostly
        implicit in the case folder. In the refactored architecture we make it
        explicit early because SimulationState is meant to travel across services.
        """
        return f"run-{uuid4().hex[:12]}"