# Mutable domain model representing the current lifecycle state of one simulation run.
#
# This module defines:
# - the high-level simulation phases,
# - the status of individual workflow steps,
# - lightweight references to artifacts,
# - the central SimulationState object passed between application services.
#
# SimulationState is the shared state carrier of the refactored workflow.
# It answers questions such as:
# - which run is this,
# - which phase are we in,
# - which steps have already completed,
# - which artifacts and results are currently known,
# - whether the current state is valid,
# - and why it may be invalid.
#
# It also contains a helper for retrieving the bootstrap definition safely while
# the architecture is still being incrementally typed and migrated.

from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Any

from domain.simulation_definition import SimulationBootstrap


class SimulationPhase(str, Enum):
    """
    High-level phase of the whole simulation run.

    The refactored code needs one common vocabulary for answering:
        "Where in the overall life cycle is the run right now?"

    This is intentionally broader than individual step statuses.
    For example:
        - geometry preparation may already be done
        - but the whole run may still only be in PREPROCESSING

    Why `str, Enum`:
    ----------------
    Inheriting from `str` makes the enum easier to log, serialize to JSON,
    store in files, or compare with external textual values.
    So each enum item has:
        - a Python enum name, e.g. `SimulationPhase.BOOTSTRAPPING`
        - a serialized string value, e.g. `"bootstrapping"`

    BOOTSTRAPPING = "bootstrapping"
        Earliest startup stage.
        The system is still loading the run definition and building the initial
        execution context.

        Typical activities here:
            - reading XML input files
            - identifying MED / meteo / family / material files
            - deriving workspace paths
            - creating the initial SimulationState object

        This does NOT yet mean geometry or coupling is initialized.
        It only means:
            "we are assembling what is needed to begin"

    PREPROCESSING = "preprocessing"
        The run definition is already loaded, and the system is now preparing
        the computational environment before the main solver run.

        Typical activities:
            - geometry preparation
            - file generation
            - descriptor writing
            - validation of inputs
            - preparation of solver working folders

    GEOMETRY_INITIALIZED = "geometry_initialized"
        The geometry-related preparation has completed successfully.

        Typical meaning:
            - geometry input has been read or transformed
            - the structures needed by downstream services now exist
            - geometry_service has finished its work

        This phase is useful if geometry preparation is a major milestone
        of the workflow and should be visible independently.

    COUPLING_INITIALIZED = "coupling_initialized"
        The coupling-related preparation has completed successfully.

        Typical meaning:
            - Solene / Saturne exchange preparation is ready
            - shared files or interfaces are prepared
            - coupling_service has finished its initialization stage

    RUNNING = "running"
        The actual computational execution is ongoing.

        Typical meaning:
            - one or more solvers are currently being executed
            - time steps are being processed
            - iterative simulation work is in progress

    POSTPROCESSING = "postprocessing"
        The main computational run has ended, and the system is now extracting,
        transforming, exporting, or organizing results.

        Typical activities:
            - result collection
            - parsing output files
            - exporting VTU or other artifacts
            - assembling result summaries
    """
    BOOTSTRAPPING = "bootstrapping"
    GEOMETRY_INITIALIZED = "geometry_initialized"
    FAMILLES_EXTRACTED = "familles_extracted"
    GEOMETRY_BUILT = "geometry_built"
    SOLENE_ENVIRONMENT_READY = "solene_environment_ready"
    SOLENE_RUNTIME_PREPARED = "solene_runtime_prepared"
    AIR_MODEL_EXECUTED = "air_model_executed"
    COUPLING_INITIALIZED = "coupling_initialized"
    SATUNE_INITIALIZED = "saturne_initialized"
    RUNNING = "running"
    POSTPROCESSING = "postprocessing"


class StepStatus(str, Enum):
    """
    Status of one individual workflow step.

    Why this enum exists:
    ---------------------
    `SimulationPhase` describes the overall run.
    `StepStatus` describes the state of one concrete sub-process.

    Example:
        - overall phase may be PREPROCESSING
        - `geometry_initialization` may be DONE
        - `coupling_initialization` may still be IN_PROGRESS

    Meaning of individual values:
    -----------------------------

    NOT_STARTED = "not_started"
        The step has not yet begun.

    IN_PROGRESS = "in_progress"
        The step is currently being executed.

    DONE = "done"
        The step finished successfully.

    FAILED = "failed"
        The step was attempted but did not complete successfully.

    INVALID = "invalid"
        The step is not currently valid or usable, even if some artifact exists.

        This is useful when:
            - an artifact was created but later determined inconsistent
            - a prior step changed and invalidated downstream outputs
            - restart logic determines a previously completed step is no longer trustworthy
    """
    NOT_STARTED = "not_started"
    IN_PROGRESS = "in_progress"
    DONE = "done"
    FAILED = "failed"
    INVALID = "invalid"


@dataclass
class ArtifactRef:
    """
    Lightweight reference to an artifact produced or consumed during the run.

    Why this class exists:
    ----------------------
    The state object should not necessarily store the full file contents or large
    parsed payloads directly. Often it is enough to track:
        - what the artifact is called
        - where it lives
        - what category it belongs to
        - whether it is currently considered valid

    Parameters:
    -----------
    name:
        Logical artifact name used inside the application.
        Examples:
            - "solene_descriptor"
            - "saturne_listing"
            - "geometry_med"
            - "comfort_results"

    path:
        Filesystem path to the artifact.

    artifact_type:
        Technical or semantic category of artifact.
        Examples:
            - "input"
            - "generated_input"
            - "output"
            - "report"
            - "mesh"
            - "result_file"

        This helps distinguish different artifact families without hardcoding
        everything into the key name.

    is_valid:
        Whether the artifact is currently considered valid for use.

        Important:
        ----------
        Existence on disk and validity are not the same thing.
        A file may exist but still be outdated, incomplete, or invalidated by
        changes in configuration or previous steps.
    """
    name: str
    path: Path
    artifact_type: str
    is_valid: bool = True


@dataclass
class SimulationState:
    """
    Mutable domain object representing the current state of one simulation run.

    This is the central "life-cycle state" object passed among application services.

    It is meant to answer questions such as:
        - Which run is this?
        - In which overall phase are we?
        - Which steps are done / running / failed?
        - Which artifacts currently exist?
        - Which results are already available?
        - Is the current state valid?
        - If not, why not?

    Why this object is important:
    -----------------------------
    In the legacy script-based workflow, much of this information is implicit:
        - spread across variables
        - encoded by folder contents
        - inferred from side effects
        - not represented by one shared object

    In the refactored architecture, this object becomes the shared state carrier
    across services such as:
        - geometry_service.py
        - coupling_service.py
        - results_service.py
        - restart_service.py

    Parameters:
    -----------

    run_id:
        Unique identifier of this specific run instance.

        Purpose:
            - distinguish one run from another
            - support logging and diagnostics
            - support restart/history tracking

        Example:
            "run-2026-08-21-001"

    workspace:
        Root workspace or case directory used by the current run.

        This is the main filesystem anchor for the simulation execution context.

    configuration_fingerprint:
        Stable identifier representing the effective input configuration.

        Why useful:
            - detect whether current artifacts belong to the same config
            - support restart validity checks
            - detect when previously produced outputs should be invalidated

        This is usually derived from the loaded input definition.

    phase:
        Current high-level phase of the run, using `SimulationPhase`.

        This answers the broad question:
            "Where are we in the overall life cycle?"

    definition:
        Reference to the static run definition loaded at startup.

        This may later point to a typed object such as `SimulationDefinition`
        rather than `Any`.

        Why it exists:
            The dynamic run state should still know what it was created from,
            but that static definition is conceptually different from the mutable
            state of execution.

    geometry_initialization:
        Status of the geometry preparation step.

        Updated by geometry-related services.

    coupling_initialization:
        Status of the coupling preparation step.

        Updated by coupling-related services.

    solene_run:
        Status of the Solene execution step.

    saturne_run:
        Status of the Code_Saturne execution step.

    postprocessing:
        Status of the postprocessing step.

    geometry_initialized:
        Convenience boolean flag indicating whether geometry preparation has been
        completed and accepted as usable.

        Why keep this if we already have `geometry_initialization`:
            It can be useful as a simpler compatibility flag for services that only
            need a yes/no answer rather than a full step state.

        Long-term note:
            In a stricter design, this may later become derivable from the step
            status instead of stored separately.

    coupling_initialized:
        Same idea as `geometry_initialized`, but for coupling preparation.

    artifacts:
        Mapping of logical artifact keys to `ArtifactRef` objects.

        Purpose:
            track the currently known files/resources of the run.

        Example keys:
            - "geometry_med"
            - "solene_input_descriptor"
            - "saturne_listing"
            - "comfort_output"

    results:
        Mapping of logical result names to already parsed or assembled results.

        Kept as `dict[str, Any]` for now because the exact result model is likely
        still evolving.

        Long-term direction:
            replace `Any` with typed result models once the result structure is stable.

    is_valid:
        Global state validity flag.

        This gives a quick answer to:
            "Can this run state currently be trusted as consistent?"

    invalid_reasons:
        Human-readable explanation of why the state is invalid.

        This is useful for:
            - debugging
            - restart decisions
            - reporting to operators/developers
    """
    run_id: str
    workspace: Path
    configuration_fingerprint: str
    phase: SimulationPhase

    # Reference to the static run definition loaded during bootstrap.
    # This is intentionally broad for now (`Any`) because the exact shape of the
    # startup definition may still evolve during refactoring.
    definition: Any

    # State of major lifecycle steps.
    # These are more granular than `phase` and allow services to update their own
    # execution status without losing visibility of the overall run stage.
    bootstrapping: StepStatus = StepStatus.NOT_STARTED
    geometry_initialization: StepStatus = StepStatus.NOT_STARTED
    familles_extraction: StepStatus = StepStatus.NOT_STARTED
    geometry_building: StepStatus = StepStatus.NOT_STARTED
    solene_environment_creation: StepStatus = StepStatus.NOT_STARTED
    solene_shared_preparation: StepStatus = StepStatus.NOT_STARTED
    air_model_execution: StepStatus = StepStatus.NOT_STARTED
    coupling_initialization: StepStatus = StepStatus.NOT_STARTED
    solene_run: StepStatus = StepStatus.NOT_STARTED
    saturne_run: StepStatus = StepStatus.NOT_STARTED
    postprocessing: StepStatus = StepStatus.NOT_STARTED

    # Known artifacts and results at the current moment of the run.
    # `default_factory=dict` is used so each SimulationState gets its own
    # independent dictionary instead of sharing one mutable default.
    artifacts: dict[str, ArtifactRef] = field(default_factory=dict)
    results: dict[str, Any] = field(default_factory=dict)

    # state validity / diagnostics
    is_valid: bool = True
    invalid_reasons: list[str] = field(default_factory=list)

    def require_bootstrap_definition(self) -> SimulationBootstrap:
        if not isinstance(self.definition, SimulationBootstrap):
            raise TypeError(
                "SimulationState.definition must be SimulationBootstrap."
            )
        return self.definition
    
    def set_phase(self, new_phase: SimulationPhase) -> None:
        if self.phase != new_phase:
            self.phase = new_phase

    def set_step_status(self, step_name: str, new_status: StepStatus) -> None:
        if not hasattr(self, step_name):
            raise AttributeError(f"SimulationState has no attribute '{step_name}'.")

        old_status = getattr(self, step_name)
        if not isinstance(old_status, StepStatus):
            raise TypeError(
                f"Attribute '{step_name}' is not a StepStatus field."
            )

        if old_status != new_status:
            print(f"[STATE] {step_name}: {new_status.value}")
            setattr(self, step_name, new_status)

    def set_validity(self, is_valid: bool, reason: str | None = None) -> None:
        if self.is_valid != is_valid:
            print(f"[STATE] is_valid: {self.is_valid} -> {is_valid}")
            self.is_valid = is_valid

        if reason is not None:
            self.invalid_reasons.append(reason)
            print(f"[STATE] invalid_reason: {reason}")