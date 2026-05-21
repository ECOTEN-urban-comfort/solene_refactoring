# Application service orchestrating the geometry workflow of the simulation.
#
# This module is responsible for:
# - validating that geometry-related input files and artifacts are present,
# - delegating technical geometry preparation to the geometry gateway,
# - registering staged geometry artifacts in SimulationState,
# - triggering the first legacy geometry extraction step,
# - storing extracted geometry results in SimulationState,
# - updating geometry lifecycle flags, step statuses, and run phase.
#
# In other words, this module owns the application-level geometry use case,
# while technical file handling and legacy extraction details are delegated to
# infrastructure adapters behind the geometry gateway.
#
# This service does not itself parse MED files or XML family/material files.
# It coordinates the workflow and maintains the state transitions around them.

from pathlib import Path
from application.ports.geometry_gateway import GeometryGateway
from domain.artifact_keys import (
    FAMILLE_XML,
    GEOMETRY_MED,
    MATERIAU_XML,
    STAGED_FAMILLE_XML,
    STAGED_GEOMETRY_MED,
    STAGED_MATERIAU_XML,
    PREPARED_INPUT_ARTIFACT,
    PREPARED_GEOMETRY_INPUTS,
    LEGACY_EXTRACTED_GEOMETRY,
)
from domain.simulation_state import (
    ArtifactRef,
    SimulationPhase,
    SimulationState,
    StepStatus,
)


class GeometryService:
    """
    Application-level geometry initialization orchestration.

    This service now has two responsibilities:
        1. validate geometry-related prerequisites
        2. call the technical geometry-preparation gateway

    What is still NOT done here:
        - MED parsing
        - family/material XML parsing into domain objects
        - geometry extraction/reconstruction
    """

    def __init__(self, gateway: GeometryGateway) -> None:
        self.gateway = gateway

    def initialize(self, state: SimulationState) -> SimulationState:
        """
        Validate geometry-related prerequisites and move the run into the
        GEOMETRY_INITIALIZED phase if successful.

        The state object is mutated in place and also returned for convenience.
        """
        state.geometry_initialization = StepStatus.IN_PROGRESS

        try:
            bootstrap = state.require_bootstrap_definition()

            # Validate the core input files required before actual geometry work.
            self._require_existing_file(
                bootstrap.input_files.med_file,
                "Missing MED geometry file.",
            )
            self._require_existing_file(
                bootstrap.input_files.famille_file,
                "Missing famille.xml file.",
            )
            self._require_existing_file(
                bootstrap.input_files.materiau_file,
                "Missing materiau.xml file.",
            )

            # Validate that the runtime state still knows these artifacts and that
            # they are currently considered valid.
            self._require_valid_artifact(state, GEOMETRY_MED)
            self._require_valid_artifact(state, FAMILLE_XML)
            self._require_valid_artifact(state, MATERIAU_XML)

            prepared = self.gateway.prepare_inputs(state)

            # Register newly created staged artifacts.
            state.artifacts[STAGED_GEOMETRY_MED] = ArtifactRef(
                name=STAGED_GEOMETRY_MED,
                path=prepared.staged_med_file,
                artifact_type=PREPARED_INPUT_ARTIFACT,
            )
            state.artifacts[STAGED_FAMILLE_XML] = ArtifactRef(
                name=STAGED_FAMILLE_XML,
                path=prepared.staged_famille_file,
                artifact_type=PREPARED_INPUT_ARTIFACT,
            )
            state.artifacts[STAGED_MATERIAU_XML] = ArtifactRef(
                name=STAGED_MATERIAU_XML,
                path=prepared.staged_materiau_file,
                artifact_type=PREPARED_INPUT_ARTIFACT,
            )

            # Store the technical preparation result for the next adapter step.
            state.results[PREPARED_GEOMETRY_INPUTS] = prepared

        except Exception as exc:
            state.geometry_initialization = StepStatus.FAILED
            state.geometry_initialized = False
            state.phase = SimulationPhase.FAILED
            state.is_valid = False
            state.invalid_reasons.append(str(exc))
            return state

        state.geometry_initialization = StepStatus.DONE
        state.geometry_initialized = True
        state.phase = SimulationPhase.GEOMETRY_INITIALIZED
        return state
    
    def extract_legacy_geometry(self, state: SimulationState) -> SimulationState:
        state.geometry_extraction = StepStatus.IN_PROGRESS

        try:
            self._require_geometry_initialized(state)

            extracted = self.gateway.extract(state)

            state.results[LEGACY_EXTRACTED_GEOMETRY] = extracted

        except Exception as exc:
            state.geometry_extraction = StepStatus.FAILED
            state.geometry_extracted = False
            state.phase = SimulationPhase.FAILED
            state.is_valid = False
            state.invalid_reasons.append(str(exc))
            return state

        state.geometry_extraction = StepStatus.DONE
        state.geometry_extracted = True
        state.phase = SimulationPhase.GEOMETRY_EXTRACTED
        return state

    def _require_existing_file(self, path: Path, message: str) -> None:
        """
        Ensure that a required geometry-related input file exists on disk.

        This is the smallest safe extraction from the old startup flow:
        before any real geometry import or reconstruction can happen, the files
        must be present and addressable.
        """
        if not path.is_file():
            raise FileNotFoundError(f"{message} Path: {path}")

    def _require_valid_artifact(
        self,
        state: SimulationState,
        artifact_key: str,
    ) -> None:
        """
        Ensure the required artifact exists in the state registry and is still valid.

        Why this check exists:
        ----------------------
        The runtime session initializer already registers the known input artifacts.
        Geometry initialization should not depend only on raw filesystem presence,
        but also on the stateful artifact registry that the new architecture uses.
        """
        artifact = state.artifacts.get(artifact_key)

        if artifact is None:
            raise ValueError(
                f"Required artifact '{artifact_key}' is not registered in SimulationState."
            )

        if not artifact.is_valid:
            raise ValueError(
                f"Required artifact '{artifact_key}' is present but marked invalid."
            )

        if not artifact.path.is_file():
            raise FileNotFoundError(
                f"Artifact '{artifact_key}' points to a missing file: {artifact.path}"
            )
        
    def _require_geometry_initialized(self, state: SimulationState) -> None:
        """
        Ensure the previous application milestone completed successfully.

        The extraction step depends on the technical staging prepared by the
        previous geometry-initialization boundary.
        """
        if not state.geometry_initialized:
            raise ValueError(
                "Geometry extraction cannot run before geometry initialization succeeds."
            )

        prepared = state.results.get(PREPARED_GEOMETRY_INPUTS)
        if prepared is None:
            raise ValueError(
                "Prepared geometry inputs are missing from SimulationState.results."
            )