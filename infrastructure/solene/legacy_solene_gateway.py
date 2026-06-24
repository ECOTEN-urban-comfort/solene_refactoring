from pathlib import Path

from application.ports.solene_gateway import SoleneGateway
from domain.artifact_keys import LEGACY_SOLENE_GEOMETRY
from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment, SoleneExportArtifacts

from infrastructure.solene.sol_command import SolCommand
from infrastructure.solene.data import Data
from infrastructure.solene.sol_env import SolEnv
from infrastructure.solene.sol_file import write_cir
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

    def create_environment(self, state: SimulationState) -> LegacySoleneEnvironment:
        solene_geometry = self._require_solene_geometry(state)
        bootstrap = state.require_bootstrap_definition()

        surface_model_profile = bootstrap.surface_model
        surface_model_name = bootstrap.settings.surface_model

        sol_command = SolCommand(
            str(bootstrap.paths.simul_sol_dir),
            bootstrap.paths.case_name,
            surface_model_profile,
            tools=bootstrap.external_tools,
        )

        time_step, meteo_list, meteo = self._build_time_step_and_meteo(
            bootstrap=bootstrap,
            sol_command=sol_command,
        )

        scene_cir_path = Path(str(sol_command.scene_cir) + ".cir")
        masque_cir_path = Path(str(sol_command.masque_cir) + ".cir")

        if not (scene_cir_path.is_file() and masque_cir_path.is_file()):
            if solene_geometry.geom_sol is None:
                raise ValueError(
                    "Cannot export Solene scene CIR because geom_sol is missing."
                )

            if solene_geometry.geom_sol_masque is None:
                raise ValueError(
                    "Cannot export Solene mask CIR because geom_sol_masque is missing."
                )

            write_cir(
                name=sol_command.masque_cir,
                geom=solene_geometry.geom_sol_masque,
                faces=True,
            )
            write_cir(
                name=sol_command.scene_cir,
                geom=solene_geometry.geom_sol,
                faces=False,
            )

        export_artifacts = SoleneExportArtifacts(
            scene_cir=scene_cir_path,
            masque_cir=masque_cir_path,
        )

        resul_sol = Data(geom=solene_geometry.geom_sol, type="2D")
        resul_sat = Data(geom=solene_geometry.geom_med, type="3D")

        sol_env = SolEnv(
            sol_command,
            solene_geometry.geom_sol,
            surface_model_name,
            data=resul_sol,
            timeStep=time_step,
            familles=solene_geometry.extracted_geometry.familles,
        )

        if meteo_list:
            sol_env.definir_meteo_liste(meteo_list)

        return LegacySoleneEnvironment(
            solene_geometry=solene_geometry,
            export_artifacts=export_artifacts,
            sol_command=sol_command,
            resul_sol=resul_sol,
            resul_sat=resul_sat,
            sol_env=sol_env,
            time_step=time_step,
            meteo_list=meteo_list,
            meteo=meteo,
        )
    
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

        meteo_all = parse_meteo_file(
            bootstrap.input_files.meteo_file,
            bootstrap.settings.meteo_file_type,
        )

        for heure_s in time_step.liste_ts:
            meteo_list.append(meteo_all[int(heure_s / time_step.ts_s) - 1])

        for i, suffix in enumerate(time_step.liste_ts_sol):
            meteo[suffix] = meteo_list[i]

        if meteo_list:
            sol_command.definir_meteo_dic(meteo_list)

        return time_step, meteo_list, meteo

    def _parse_meteo_file(self, fichier_meteo: str, type_meteo: str):
        if type_meteo == "RT":
            return meteo_obj.parser_fichier_meteo_RT(fichier_meteo)
        if type_meteo == "ONEVU":
            return meteo_obj.parser_fichier_meteo_ONEVU(fichier_meteo)
        if type_meteo == "HEPIA":
            return meteo_obj.parser_fichier_meteo_HEPIA(fichier_meteo)
        if type_meteo == "Khaled":
            return meteo_obj.parser_fichier_meteo_khaled(fichier_meteo)
        if type_meteo == "ILYES":
            return meteo_obj.parser_fichier_meteo_ILYES(fichier_meteo)

        raise ValueError(f"Unsupported meteo file type: {type_meteo}")

    def _require_solene_geometry(self, state: SimulationState):
        solene_geometry = state.results.get(LEGACY_SOLENE_GEOMETRY)
        if solene_geometry is None:
            raise ValueError(
                "Solene-side geometry is missing; build it before creating the Solene environment."
            )
        return solene_geometry

    def prepare_shared_runtime(
        self,
        environment: LegacySoleneEnvironment,
    ) -> None:
        """
        Execute Solene runtime preparation common to all air models.
        """
        sol_command = environment.sol_command
        sol_env = environment.sol_env

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
        solene_geometry = state.results.get(LEGACY_SOLENE_GEOMETRY)
        if solene_geometry is None:
            raise ValueError(
                "Solene-side geometry is missing; build it before creating the Solene environment."
            )
        return solene_geometry