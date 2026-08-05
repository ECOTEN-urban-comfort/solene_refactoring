from pathlib import Path
from typing import Tuple, Dict

from application.ports.solene_gateway import SoleneGateway
from domain.artifact_keys import BUILT_GEOMETRY_ARTIFACTS
from domain.simulation_definition import SimulationBootstrap
from domain.simulation_state import SimulationState
from domain.geometry import SoleneGeometryArtifacts

from infrastructure.solene.famille import Familles
from infrastructure.solene.hdfFile import CplFile
from infrastructure.solene.sol_command import SolCommand
from infrastructure.saturne.sat_command import SatCommand
from infrastructure.solene.data import Data
from infrastructure.solene.sol_env import SolEnv
from infrastructure.solene.timeStep import TimeStep
import infrastructure.solene.meteo as meteo_obj

class LegacySoleneGateway(SoleneGateway):
    """
    Legacy-backed implementation of the first Solene runtime setup step.

    What this transfers from the old code:
    -------------------------------------
    - `exporter_geom_solene()`
    - `self.resul_sol = Data(...)`
    - `self.resul_sat = Data(...)`
    - `self.solEnv = SolEnv(...)`
    - `self.solEnv.definir_meteo_liste(self.meteo_liste)`

    What it intentionally does NOT do yet:
    --------------------------------------
    - actual Solene simulation launch,
    - descriptor generation,
    - result extraction,
    - coupling with Saturne.
    """

    def create_environment(self, bootstrap: SimulationBootstrap, geometry: SoleneGeometryArtifacts, families: Familles) -> Tuple[SolCommand, SatCommand, SolEnv, TimeStep, Dict[str, Dict[str, float]]]:

        surface_model_profile = bootstrap.surface_model

        sol_command = SolCommand(
            str(bootstrap.paths.simul_sol_dir),
            bootstrap.paths.case_name,
            surface_model_profile,
            tools=bootstrap.external_tools,
        )

        sat_command = SatCommand(
            str(bootstrap.paths.simul_sat_dir),
            bootstrap.paths.case_name,
            code_saturne_executable = bootstrap.saturne_tools.code_saturne,
            tmp_saturne_dir = bootstrap.paths.tmp_saturne_dir,
        )

        time_step, meteo_list = self._build_time_step_and_meteo(
            bootstrap=bootstrap,
            sol_command=sol_command,
        )

        geom_sol = self._load_geom(geometry.geom_sol_cpl, "geom_sol")
        #geom_med = self._load_geom(geometry.geom_med_cpl, "geom_med")

        resul_sol = Data(geom=geom_sol, type="2D")
        #resul_sat = Data(geom=geom_med, type="3D")

        sol_env = SolEnv(
            sol_command,
            geom_sol,
            bootstrap.settings.surface_model,
            data=resul_sol,
            timeStep=time_step,
            familles=families,
        )

        if meteo_list:
            sol_env.definir_meteo_liste(meteo_list)

        return sol_command, sat_command, sol_env, time_step, meteo_list
    
    @staticmethod
    def _load_geom(path: Path, nom: str):
        geom_cpl = CplFile(str(path))
        geom_cpl.charger_geom()
        geom = geom_cpl.geom
        geom.nom = nom
        return geom
    
    def _build_time_step_and_meteo(self, bootstrap, sol_command):
        time_step = TimeStep()
        meteo_list = []
        meteo = {}

        interval = bootstrap.settings.interval
        if interval:
            time_step.definir_liste_ts(interval)
            sol_command.pas_de_temps = time_step.ts_s
            sol_command.definir_liste_jours(time_step.liste_jours)
            sol_command.liste_ts_sol = time_step.liste_ts_sol

        meteo_type = getattr(bootstrap.settings, "meteo_file_type", "ONEVU")
        meteo_all = meteo_obj.parse_meteo_file(
            bootstrap.input_files.meteo_file,
            meteo_type,
        )

        for heure_s in time_step.liste_ts:
            meteo_list.append(meteo_all[int(heure_s / time_step.ts_s) - 1])

        for i, suffix in enumerate(time_step.liste_ts_sol):
            meteo[suffix] = meteo_list[i]

        if meteo_list:
            sol_command.definir_meteo_dic(meteo_list)

        return time_step, meteo_list

    def prepare_shared_runtime(
        self,
        sol_command: SolCommand,
        sol_env: SolEnv,
    ) -> None:
        """
        Execute Solene runtime preparation common to all air models.
        """
        sol_command = sol_command
        sol_env = sol_env

        sol_env.creer_descripteur_solaires()
        sol_env.creer_descripteur_veg()

        # pokud už je meteo list navázaný, tady jen využití
        sol_command.calculer_luminance_ciel()
        sol_command.calculer_flux_solaires()
        sol_command.calculer_fac_form()
        sol_command.calculer_fac_form_ciel()
        sol_command.calculer_radiosite()

        return

    def _require_solene_geometry(self, state: SimulationState):
        solene_geometry = state.geometry_ref.get(BUILD_GEOMETRY_ARTIFACTS)
        if solene_geometry is None:
            raise ValueError(
                "Solene-side geometry is missing; build it before creating the Solene environment."
            )
        return solene_geometry