from domain.simulation_state import SimulationState
from domain.simulation_state import StepStatus
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS


class CouplingService:

    def __init__(self, gateway):
        self.gateway = gateway

    def initialize_exchange(
        self,
        state: SimulationState,
    ) -> SimulationState:
        try:
            state.set_step_status(
                "coupling_initial_exchange",
                StepStatus.IN_PROGRESS,
            )

            bootstrap = state.require_bootstrap_definition()
            settings = bootstrap.settings

            time_step = state.time_step

            if time_step is None:
                raise ValueError("TimeStep is not initialized.")

            if state.sol_command is None:
                raise ValueError("SolCommand is not initialized.")

            if state.sat_command is None:
                raise ValueError("SatCommand is not initialized.")

            if state.sol_env is None:
                raise ValueError("SolEnv is not initialized.")

            b_coupl = time_step.n_ts - settings.ts_coupl

            if b_coupl < 1 or b_coupl >= time_step.n_ts:
                raise ValueError(
                    "Invalid coupling start index: "
                    f"n_ts={time_step.n_ts}, "
                    f"ts_coupl={settings.ts_coupl}, "
                    f"b_coupl={b_coupl}."
                )

            previous_ts = time_step.liste_ts_sol[
                b_coupl - 1
            ]

            current_ts = time_step.liste_ts_sol[
                b_coupl
            ]

            # Legacy:
            # chemin_hc   = sim.SolCommand.var['hc']
            # chemin_Tair = sim.SolCommand.var['Tair']
            # chemin_HR   = sim.SolCommand.var['HR']
            # chemin_v    = sim.SolCommand.carac['v']

            base_paths = {
                "hc": state.sol_command.var["hc"],
                "Tair": state.sol_command.var["Tair"],
                "HR": state.sol_command.var["HR"],
                "v": state.sol_command.carac["v"],
            }

            # These values must survive initialize_exchange -> run_step -> finalize,
            # whereas they were local variables in the legacy coupling() function.
            state.results["coupling_base_paths"] = base_paths

            geometry = state.geometry_ref[
                BUILT_GEOMETRY_ARTIFACTS
            ]

            self.gateway.initialize_exchange(
                sol_command=state.sol_command,
                sat_command=state.sat_command,
                sol_env=state.sol_env,
                geometry=geometry,
                step_index=b_coupl,
                previous_ts=previous_ts,
                current_ts=current_ts,
                processors=settings.cores_used,
                base_paths=base_paths,
            )

        except Exception as exc:
            state.set_step_status(
                "coupling_initial_exchange",
                StepStatus.FAILED,
            )
            state.set_validity(False, str(exc))
            return state

        state.set_step_status(
            "coupling_initial_exchange",
            StepStatus.DONE,
        )

        return state

    def run_loop(
        self,
        state: SimulationState,
    ) -> SimulationState:
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

        base_paths = state.results.get(
            "coupling_base_paths"
        )

        if base_paths is None:
            raise ValueError(
                "Coupling base paths are not initialized. "
                "initialize_exchange() must run first."
            )

        previous_ts = time_step.liste_ts_sol[
            step_index - 1
        ]

        current_ts = time_step.liste_ts_sol[
            step_index
        ]

        self.gateway.run_step(
            sol_command=state.sol_command,
            sat_command=state.sat_command,
            sol_env=state.sol_env,
            meteo=state.meteo[step_index],
            step_index=step_index,
            previous_ts=previous_ts,
            current_ts=current_ts,
            iterations=settings.iter_foll,
            processors=settings.cores_used,
            base_paths=base_paths,
        )

        return state