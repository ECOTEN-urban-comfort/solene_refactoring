from dataclasses import dataclass
import numpy as np

from domain.air_model_definition import AirModelDefinition
from domain.solene import LegacySoleneEnvironment
from infrastructure.solene.air_models.formulas import (
    wind_speed_at_height,
    ashrae_hc,
    denby_hc,
    vehrencamp_hc,
    nusselt_hc,
)
from infrastructure.solene.sol_file import read_val, write_val


@dataclass(frozen=True)
class AirModelRunConfig:
    ts_coupl: int
    temp_init: float
    hc_init: float
    z_ref_m: float = 3.0
    z_target_m: float = 10.0
    z0_m: float = 0.4


class AirModelRunner:
    def run(
        self,
        environment: LegacySoleneEnvironment,
        air_model: AirModelDefinition,
        config: AirModelRunConfig,
    ) -> None:
        if air_model.hc_mode == "scalar":
            self._run_scalar_hc_model(environment, air_model, config)
            return

        if air_model.hc_mode == "per_triangle":
            self._run_per_triangle_hc_model(environment, air_model, config)
            return

        raise ValueError(f"Unsupported hc_mode: {air_model.hc_mode}")

    def _run_scalar_hc_model(
        self,
        environment: LegacySoleneEnvironment,
        air_model: AirModelDefinition,
        config: AirModelRunConfig,
    ) -> None:
        time_step = environment.time_step
        meteo_list = environment.meteo_list
        meteo = environment.meteo
        sol_env = environment.sol_env
        sol_command = environment.sol_command

        if time_step is None:
            raise ValueError("Air model execution requires initialized TimeStep.")

        b_coupl = time_step.n_ts - config.ts_coupl

        sol_env.creer_param_simulation_ts(T_init=config.temp_init, hc=config.hc_init)
        sol_env.creer_evaporation()

        v0 = wind_speed_at_height(
            v_ref=meteo_list[0]["v"],
            z_ref=config.z_ref_m,
            z_target=config.z_target_m,
            z0=config.z0_m,
        )
        hc0 = self._compute_scalar_hc(air_model, v0)

        sol_env.creer_param_simulation_ts(hc=hc0)
        sol_env.definir_meteo(0, veg=True)
        sol_command.simulation_Ts_EnergieBat_new(
            "init",
            time_step.liste_ts_sol[0],
            simulation_batiment=False,
            simulation_vegetation=True,
            meteo=True,
            terminal=True,
        )

        for i in range(1, b_coupl):
            avant = time_step.liste_ts_sol[i - 1]
            apres = time_step.liste_ts_sol[i]

            vi = wind_speed_at_height(
                v_ref=meteo_list[i]["v"],
                z_ref=config.z_ref_m,
                z_target=config.z_target_m,
                z0=config.z0_m,
            )
            hc = self._compute_scalar_hc(air_model, vi)

            sol_env.creer_param_simulation_ts(hc=hc)
            sol_env.definir_meteo(i, veg=True)
            sol_command.simulation_Ts_EnergieBat_new(
                avant,
                apres,
                simulation_batiment=False,
                simulation_vegetation=True,
                meteo=True,
                terminal=True,
            )

    def _run_per_triangle_hc_model(
        self,
        environment: LegacySoleneEnvironment,
        air_model: AirModelDefinition,
        config: AirModelRunConfig,
    ) -> None:
        time_step = environment.time_step
        meteo_list = environment.meteo_list
        meteo = environment.meteo
        sol_env = environment.sol_env
        sol_command = environment.sol_command

        if time_step is None:
            raise ValueError("Air model execution requires initialized TimeStep.")

        b_coupl = time_step.n_ts - config.ts_coupl

        sol_env.creer_param_simulation_ts(T_init=config.temp_init, hc=config.hc_init)
        sol_env.creer_evaporation()

        sol_env.creer_param_simulation_ts(hc=config.hc_init)
        sol_env.definir_meteo(0, veg=True)
        sol_command.simulation_Ts_EnergieBat_new(
            "init",
            time_step.liste_ts_sol[0],
            simulation_batiment=False,
            simulation_vegetation=True,
            meteo=True,
            terminal=True,
        )

        for i in range(1, b_coupl):
            avant = time_step.liste_ts_sol[i - 1]
            apres = time_step.liste_ts_sol[i]

            vi = wind_speed_at_height(
                v_ref=meteo_list[i]["v"],
                z_ref=config.z_ref_m,
                z_target=config.z_target_m,
                z0=config.z0_m,
            )
            tair = meteo[avant]["T"]
            tsurf = read_val(f"{sol_command.var['Tse']}_{avant}.val", geom=sol_env.geom)

            hc_values = np.zeros(sol_env.geom.n_triangles)
            for idx in range(sol_env.geom.n_triangles):
                if air_model.name == "vehrencamp":
                    hc_values[idx] = vehrencamp_hc(float(tsurf[idx]), tair, vi)
                elif air_model.name == "nusselt":
                    hc_values[idx] = nusselt_hc(float(tsurf[idx]), tair, vi, characteristic_length=1.0)
                else:
                    raise ValueError(f"Unsupported per-triangle air model: {air_model.name}")

            write_val(sol_command.var["hc"], sol_env.geom, hc_values)
            sol_env.definir_meteo(i, veg=True)
            sol_command.simulation_Ts_EnergieBat_new(
                avant,
                apres,
                simulation_batiment=False,
                simulation_vegetation=True,
                meteo=True,
                terminal=True,
            )

    def _compute_scalar_hc(self, air_model: AirModelDefinition, v_air: float) -> float:
        if air_model.name == "ashrae":
            return ashrae_hc(v_air)
        if air_model.name == "denby":
            return denby_hc(v_air)
        raise ValueError(f"Unsupported scalar air model: {air_model.name}")