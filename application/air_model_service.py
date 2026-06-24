from dataclasses import dataclass

from domain.artifact_keys import LEGACY_SOLENE_ENVIRONMENT
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
            environment = state.results.get(LEGACY_SOLENE_ENVIRONMENT)
            if environment is None:
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
                environment=environment,
                air_model=air_model,
                config=config,
            )

        except Exception as exc:
            state.set_step_status("air_model_execution", StepStatus.FAILED)
            state.set_validity(False, str(exc))
            return state

        state.set_step_status("air_model_execution", StepStatus.DONE)
        state.set_phase(SimulationPhase.AIR_MODEL_EXECUTED)
        return state