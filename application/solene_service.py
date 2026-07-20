from application.ports.solene_gateway import SoleneGateway
from domain.geometry import SoleneGeometryArtifacts
from domain.simulation_state import SimulationPhase, SimulationState, StepStatus
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS

class SoleneService:
    """
    Application service orchestrating the first Solene runtime setup step.

    This service assumes that Solene-side geometry already exists in state and then:
        - delegates export + environment creation to the technical Solene gateway,
        - stores the resulting Solene environment bundle in SimulationState,
        - updates lifecycle state accordingly.
    """

    def __init__(self, gateway: SoleneGateway) -> None:
        self.gateway = gateway

    def create_environment(self, state: SimulationState) -> SimulationState:
        """
        Create the first Solene runtime environment after Solene-side geometry
        has already been built.
        """
        try:
            state.set_step_status("solene_environment_creation", StepStatus.IN_PROGRESS)

            geometry = state.geometry_ref.get(BUILT_GEOMETRY_ARTIFACTS)
            bootstrap = state.require_bootstrap_definition()
            families = state.families

            if bootstrap is None or geometry is None or families is None:
                raise ValueError(
                    "Cannot create Solene environment before Solene-side prerequisites are available."
                )

            sol_command, sat_command, sol_env, time_step, meteo = self.gateway.create_environment(bootstrap, geometry, families)
            state.sol_command = sol_command
            state.sat_command = sat_command
            state.sol_env = sol_env
            state.time_step = time_step
            state.meteo = meteo

        except Exception as exc:
            state.set_step_status("solene_environment_creation", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state
        
        state.set_step_status("solene_environment_creation", StepStatus.DONE)
        state.set_phase(SimulationPhase.SOLENE_ENVIRONMENT_READY)

        return state

    def prepare_shared_runtime(self, state: SimulationState) -> SimulationState:
        """
        Execute the shared Solene runtime preparation that is common for all
        air models and happens after environment creation.
        """
        try:
            state.set_step_status("solene_shared_preparation", StepStatus.IN_PROGRESS)
            sol_command = state.sol_command
            sol_env = state.sol_env
            if sol_command is None or sol_env is None:
                raise ValueError(
                    "Cannot prepare shared Solene runtime before environment exists."
                )

            self.gateway.prepare_shared_runtime(sol_command, sol_env)

        except Exception as exc:
            state.set_step_status("solene_shared_preparation", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state

        state.set_step_status("solene_shared_preparation", StepStatus.DONE)
        state.set_phase(SimulationPhase.SOLENE_RUNTIME_PREPARED)
        return state
    
    def _require_solene_geometry(self, state: SimulationState) -> SoleneGeometryArtifacts:
        solene_geometry = state.geometry_ref.get(BUILT_GEOMETRY_ARTIFACTS)
        if solene_geometry is None:
            raise ValueError(
                "Solene-side geometry is missing; build it before creating the Solene environment."
            )
        return solene_geometry