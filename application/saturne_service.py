from application.ports.saturne_gateway import SaturneGateway
from domain.artifact_keys import LEGACY_SOLENE_ENVIRONMENT
from domain.simulation_state import SimulationPhase, SimulationState, StepStatus


class SaturneService:
    LEGACY_SOLENE_ENVIRONMENT
       
    def __init__(self, gateway: SaturneGateway) -> None:
        self.gateway = gateway

        def initialize(self, state: SimulationState) -> SimulationState:
            try:
                state.set_step_status("saturne_initialization", StepStatus.IN_PROGRESS)

                environment = state.results.get(LEGACY_SOLENE_ENVIRONMENT)
                if environment is None:
                    raise ValueError(
                        "Cannot initialize Saturne before Solene environment exists."
                    )

                self.gateway.initialize(environment, state)

            except Exception as exc:
                state.set_step_status("saturne_initialization", StepStatus.FAILED)
                state.set_validity(False, str(exc))
                return state

            state.set_step_status("saturne_initialization", StepStatus.DONE)
            state.set_phase(SimulationPhase.SATURNE_INITIALIZED)
            return state