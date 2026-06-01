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
            solene_geometry = state.results.get(LEGACY_SOLENE_GEOMETRY)
            if solene_geometry is None:
                raise ValueError(
                    "Cannot create Solene environment before Solene-side geometry exists."
                )

            environment = self.gateway.create_environment(state)
            state.results[LEGACY_SOLENE_ENVIRONMENT] = environment
            state.solene_environment_creation = StepStatus.DONE
            state.solene_environment_ready = True
            state.phase = SimulationPhase.SOLENE_ENVIRONMENT_READY

        except Exception as exc:
            state.solene_environment_creation = StepStatus.FAILED
            state.solene_environment_ready = False
            state.phase = SimulationPhase.FAILED
            state.is_valid = False
            state.invalid_reasons.append(str(exc))
            return state

        return state