from dataclasses import dataclass
import numpy as np

from domain.simulation.air_model_definition import AirModelDefinition
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
    hc_init: float
    z_ref_m: float = 10.0
    z_target_m: float = 1.5
    z0_m: float = 0.4


class AirModelRunner:
    def run(self, sim, air_model: AirModelDefinition, config: AirModelRunConfig) -> None:
        if air_model.hc_mode == "scalar":
            self._run_scalar_hc_model(sim, air_model, config)
            return

        if air_model.hc_mode == "per_triangle":
            self._run_per_triangle_hc_model(sim, air_model, config)
            return

        raise ValueError(f"Unsupported hc_mode: {air_model.hc_mode}")
    
    def _run_scalar_hc_model(self, sim, air_model: AirModelDefinition, config: AirModelRunConfig) -> None:
        b_coupl = sim.TimeStep.n_ts - config.ts_coupl

        v0_ref = sim.meteo_liste[0]["v"]
        v0 = wind_speed_at_height(
            v_ref=v0_ref,
            z_ref=config.z_ref_m,
            z_target=config.z_target_m,
            z0=config.z0_m,
        )

        hc0 = self._compute_scalar_hc(air_model, v0)

        sim.solEnv.creer_param_simulation_ts(
            T_init=sim.solEnv.default_T_init if hasattr(sim.solEnv, "default_T_init") else None,
            hc=hc0,
        )

        for i in range(1, b_coupl):
            avant = sim.TimeStep.liste_ts_sol[i - 1]
            apres = sim.TimeStep.liste_ts_sol[i]

            v_ref = sim.meteo[avant]["v"]
            v_air = wind_speed_at_height(
                v_ref=v_ref,
                z_ref=config.z_ref_m,
                z_target=config.z_target_m,
                z0=config.z0_m,
            )

            hc = self._compute_scalar_hc(air_model, v_air)

            sim.solEnv.creer_param_simulation_ts(hc=hc)
            sim.SolCommand.simulation_Ts_EnergieBat_new(
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
    
    def _run_per_triangle_hc_model(self, sim, air_model: AirModelDefinition, config: AirModelRunConfig) -> None:
        b_coupl = sim.TimeStep.n_ts - config.ts_coupl

        sim.solEnv.creer_param_simulation_ts(hc=config.hc_init)

        for i in range(1, b_coupl):
            avant = sim.TimeStep.liste_ts_sol[i - 1]
            apres = sim.TimeStep.liste_ts_sol[i]

            v_ref = sim.meteo[avant]["v"]
            v_air = wind_speed_at_height(
                v_ref=v_ref,
                z_ref=config.z_ref_m,
                z_target=config.z_target_m,
                z0=config.z0_m,
            )

            t_air = sim.meteo[avant]["T"]
            t_surf = read_val(f"{sim.SolCommand.var['Tse']}_{avant}", geom=sim.solEnv.geom)

            hc_values = self._compute_per_triangle_hc(
                sim=sim,
                air_model=air_model,
                t_surf=t_surf,
                t_air=t_air,
                v_air=v_air,
            )

            write_val(sim.SolCommand.var["hc"], sim.solEnv.geom, hc_values)

            sim.SolCommand.simulation_Ts_EnergieBat_new(
                avant,
                apres,
                simulation_batiment=False,
                simulation_vegetation=True,
                meteo=True,
                terminal=True,
            )

    def _compute_per_triangle_hc(self, sim, air_model: AirModelDefinition, t_surf, t_air: float, v_air: float):
        values = np.zeros(sim.solEnv.geom.n_triangles)

        for i_tri in range(sim.solEnv.geom.n_triangles):
            if air_model.name == "vehrencamp":
                values[i_tri] = vehrencamp_hc(
                    t_surf=float(t_surf[i_tri]),
                    t_air=t_air,
                    v_air=v_air,
                )
            elif air_model.name == "nusselt":
                values[i_tri] = nusselt_hc(
                    t_surf=float(t_surf[i_tri]),
                    t_air=t_air,
                    v_air=v_air,
                    characteristic_length=1.0,
                )
            else:
                raise ValueError(f"Unsupported per-triangle air model: {air_model.name}")

        return values
    
