from application.ports.saturne_gateway import SaturneGateway
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS
from domain.simulation_state import SimulationPhase, SimulationState, StepStatus


class SaturneService:
       
    def __init__(self, gateway: SaturneGateway) -> None:
        self.gateway = gateway

    def initialize(self, state: SimulationState) -> SimulationState:
        try:
            state.set_step_status("saturne_initialization", StepStatus.IN_PROGRESS)
            sat_command = state.sat_command
            families = state.families
            n_proc_saturne = state.n_proc_saturne
            n_triangles = state.geometry_ref[BUILT_GEOMETRY_ARTIFACTS].n_sol_triangles
            bootstrap = state.require_bootstrap_definition()
            
            if sat_command is None or families is None or n_proc_saturne is None or n_triangles is None:
                raise ValueError(
                    "Cannot initialize Saturne before Saturne environment exists."
                )

            self.gateway.initialize(sat_command, families, n_proc_saturne, n_triangles, bootstrap)

        except Exception as exc:
            state.set_step_status("saturne_initialization", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state

        state.set_step_status("saturne_initialization", StepStatus.DONE)
        state.set_phase(SimulationPhase.SATURNE_INITIALIZED)
        return state