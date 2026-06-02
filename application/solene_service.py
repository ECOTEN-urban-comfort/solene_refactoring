from application.ports.solene_gateway import SoleneGateway
from domain.simulation_state import SimulationPhase, SimulationState, StepStatus

LEGACY_SOLENE_GEOMETRY = "legacy_solene_geometry"
LEGACY_SOLENE_ENVIRONMENT = "legacy_solene_environment"


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

            solene_geometry = state.results.get(LEGACY_SOLENE_GEOMETRY)
            if solene_geometry is None:
                raise ValueError(
                    "Cannot create Solene environment before Solene-side geometry exists."
                )

            environment = self.gateway.create_environment(state)
            state.results[LEGACY_SOLENE_ENVIRONMENT] = environment

        except Exception as exc:
            state.set_step_status("solene_environment_creation", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state
        
        state.set_step_status("solene_environment_creation", StepStatus.DONE)
        state.set_phase(SimulationPhase.SOLENE_ENVIRONMENT_READY)

        return state