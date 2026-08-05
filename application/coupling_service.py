# Governing the coupling between the different models in the simulation, such as the coupling between the CFD model and the daylight model, the coupling between the thermal model and the daylight model, etc. This includes managing the data exchange between the models, ensuring that the models are synchronized in time, and managing the dependencies between the models.
# A) Order of running of the individual models
# B) Data transfer between the models, such as transferring the temperature field from the CFD model to the daylight model, transferring the daylight factor from the daylight model to the thermal model, etc.
# C) Input and output validation between subsystems, such as checking that the output of the CFD model is in the correct format for the daylight model, checking that the output of the daylight model is in the correct format for the thermal model, etc.
# D) Synchronization of the models in time, such as ensuring that the CFD model and the daylight model are synchronized in time, ensuring that the thermal model is synchronized with the daylight model, etc.
# E) Recognizing if the coupling went wrong, such as if the CFD model failed to run, if the daylight model failed to run, if the thermal model failed to run, etc., and handling these cases appropriately, such as by logging the error, by retrying the model, by skipping the model, etc.

from domain.simulation_state import SimulationState
from domain.simulation_state import StepStatus
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS

class CouplingService:

    def run_loop(self, state: SimulationState) -> SimulationState:
        try:
            state.set_step_status(
                "coupling_loop",
                StepStatus.IN_PROGRESS,
            )

            bootstrap = state.require_bootstrap_definition()
            settings = bootstrap.settings

            time_step = state.time_step
            if time_step is None:
                raise ValueError("TimeStep is not initialized.")

            b_coupl = time_step.n_ts - settings.ts_coupl

            if b_coupl < 1 or b_coupl >= time_step.n_ts:
                raise ValueError(
                    "Invalid coupling start index: "
                    f"n_ts={time_step.n_ts}, "
                    f"ts_coupl={settings.ts_coupl}, "
                    f"b_coupl={b_coupl}."
                )

            for step_index in range(
                b_coupl + 1,
                time_step.n_ts,
            ):
                state = self.run_step(
                    state=state,
                    step_index=step_index,
                )

                if not state.is_valid:
                    return state

        except Exception as exc:
            state.set_step_status(
                "coupling_loop",
                StepStatus.FAILED,
            )
            state.set_validity(False, str(exc))
            return state

        state.set_step_status(
            "coupling_loop",
            StepStatus.DONE,
        )

        return state

    def run_step(
        self,
        state: SimulationState,
        step_index: int,
    ) -> SimulationState:
        bootstrap = state.require_bootstrap_definition()
        settings = bootstrap.settings

        time_step = state.time_step
        if time_step is None:
            raise ValueError("TimeStep is not initialized.")

        previous_ts = time_step.liste_ts_sol[step_index - 1]
        current_ts = time_step.liste_ts_sol[step_index]

        self.gateway.run_step(
            sol_command=state.sol_command,
            sat_command=state.sat_command,
            sol_env=state.sol_env,
            geometry=state.geometry_ref[BUILT_GEOMETRY_ARTIFACTS],
            meteo=state.meteo[step_index],
            previous_ts=previous_ts,
            current_ts=current_ts,
            iterations=settings.iter_foll,
            processors=settings.cores_used,
        )

        return state