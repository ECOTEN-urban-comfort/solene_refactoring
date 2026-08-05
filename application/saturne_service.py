from application.ports.saturne_gateway import SaturneGateway
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS
from domain.simulation_state import SimulationPhase, SimulationState, StepStatus


class SaturneService:
       
    def __init__(self, gateway: SaturneGateway) -> None:
        self.gateway = gateway

    def initialize(self, state: SimulationState) -> SimulationState:
        try:
            state.set_step_status(
                "saturne_initialization",
                StepStatus.IN_PROGRESS,
            )

            bootstrap = state.require_bootstrap_definition()
            settings = bootstrap.settings

            geometry = state.geometry_ref.get(BUILT_GEOMETRY_ARTIFACTS)

            if geometry is None:
                raise ValueError("Solene geometry artifacts are missing.")

            if state.sat_command is None:
                raise ValueError("SatCommand is not initialized.")

            if state.families is None:
                raise ValueError("Families are not initialized.")

            self.gateway.initialize(
                sat_command=state.sat_command,
                families=state.families,
                n_proc_saturne=settings.cores_used,
                n_triangles=geometry.n_sol_triangles,
                bootstrap=bootstrap,
            )

        except Exception as exc:
            state.set_step_status(
                "saturne_initialization",
                StepStatus.FAILED,
            )
            state.set_validity(False, str(exc))
            return state

        state.set_step_status(
            "saturne_initialization",
            StepStatus.DONE,
        )

        return state
    
    def run_initial(self, state: SimulationState) -> SimulationState:
        try:
            state.set_step_status(
                "saturne_initial_run",
                StepStatus.IN_PROGRESS,
            )

            bootstrap = state.require_bootstrap_definition()
            settings = bootstrap.settings

            sat_command = state.sat_command
            time_step = state.time_step
            meteo_list = state.meteo

            if sat_command is None:
                raise ValueError("SatCommand is not initialized.")

            if time_step is None:
                raise ValueError("TimeStep is not initialized.")

            if meteo_list is None:
                raise ValueError("Meteorological data are not initialized.")

            b_coupl = time_step.n_ts - settings.ts_coupl

            if b_coupl < 1 or b_coupl >= time_step.n_ts:
                raise ValueError(
                    "Invalid coupling start index: "
                    f"n_ts={time_step.n_ts}, "
                    f"ts_coupl={settings.ts_coupl}, "
                    f"b_coupl={b_coupl}."
                )

            self.gateway.run_initial(
                sat_command=sat_command,
                meteo=meteo_list[b_coupl],
                iterations=settings.iter_init,
            )

        except Exception as exc:
            state.set_step_status(
                "saturne_initial_run",
                StepStatus.FAILED,
            )
            state.set_validity(False, str(exc))
            return state

        state.set_step_status(
            "saturne_initial_run",
            StepStatus.DONE,
        )
        state.set_phase(SimulationPhase.SATURNE_INITIAL_RUN_DONE)

        return state

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
            sol_command = state.sol_command
            sat_command = state.sat_command
            sol_env = state.sol_env

            geometry = state.geometry_ref.get(
                BUILT_GEOMETRY_ARTIFACTS
            )

            if time_step is None:
                raise ValueError("TimeStep is not initialized.")
            if sol_command is None:
                raise ValueError("SolCommand is not initialized.")
            if sat_command is None:
                raise ValueError("SatCommand is not initialized.")
            if sol_env is None:
                raise ValueError("SolEnv is not initialized.")
            if geometry is None:
                raise ValueError("Geometry artifacts are missing.")

            b_coupl = time_step.n_ts - settings.ts_coupl
            current_ts = time_step.liste_ts_sol[b_coupl]
            previous_ts = time_step.liste_ts_sol[b_coupl - 1]

            self.gateway.initialize_exchange(
                geometry=geometry,
                sol_command=sol_command,
                sat_command=sat_command,
                sol_env=sol_env,
                time_step=time_step,
                coupling_index=b_coupl,
                previous_ts=previous_ts,
                current_ts=current_ts,
                processors=settings.cores_used,
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