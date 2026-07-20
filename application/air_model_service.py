from dataclasses import dataclass
import traceback

from domain.simulation_state import SimulationPhase, SimulationState, StepStatus
from infrastructure.solene.air_models.runner import AirModelRunConfig, AirModelRunner


@dataclass
class AirModelService:
    runner: AirModelRunner

    def run(self, state: SimulationState) -> SimulationState:
        """
        Execute the selected air-model branch on top of an already prepared
        Solene environment.
        """
        try:
            state.set_step_status("air_model_execution", StepStatus.IN_PROGRESS)

            bootstrap = state.require_bootstrap_definition()
            if state.sol_command is None or state.sol_env is None or state.time_step is None:
                raise ValueError(
                    "Cannot execute air model before Solene environment exists."
                )

            air_model = bootstrap.air_model

            config = AirModelRunConfig(
                ts_coupl=bootstrap.settings.ts_coupl,
                temp_init=bootstrap.settings.temp_init,
                hc_init=bootstrap.settings.hc_init,
            )

            self.runner.run(
                air_model=air_model,
                config=config,
                sol_command=state.sol_command,
                sol_env=state.sol_env,
                time_step=state.time_step,
                meteo=state.meteo,
            )

        except Exception as exc:
            full_tb = "".join(traceback.format_exception(type(exc), exc, exc.__traceback__))

            state.set_step_status("air_model_execution", StepStatus.FAILED)
            state.set_validity(False, full_tb)
            return state

        state.set_step_status("air_model_execution", StepStatus.DONE)
        state.set_phase(SimulationPhase.AIR_MODEL_EXECUTED)
        return state