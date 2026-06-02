from pathlib import Path

from application.ports.solene_gateway import SoleneGateway
from domain.artifact_keys import LEGACY_SOLENE_GEOMETRY, LEGACY_TIME_STEP, METEO_LIST
from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment, SoleneExportArtifacts

from infrastructure.geometry.legacy.famille import CARAC_CLASSE
from infrastructure.geometry.legacy.solCommand import SolCommand
from infrastructure.geometry.legacy.data import Data
from infrastructure.geometry.legacy.solEnv import SolEnv
from infrastructure.geometry.legacy.solFile import write_cir


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

        # Recreate the minimum Solene command/runtime context needed for .cir export.
        sol_command = SolCommand(
            str(bootstrap.paths.simul_sol_dir),
            bootstrap.paths.case_name,
        )

        scene_cir_path = Path(str(sol_command.scene_cir) + ".cir")
        masque_cir_path = Path(str(sol_command.masque_cir) + ".cir")

        # Export only if CIR files are actually missing.
        if not (scene_cir_path.is_file() and masque_cir_path.is_file()):
            if solene_geometry.geom_sol is None:
                raise ValueError(
                    "Cannot export Solene scene CIR because geom_sol is missing."
                )

            if solene_geometry.geom_sol_masque is None:
                raise ValueError(
                    "Cannot export Solene mask CIR because geom_sol_masque is missing. "
                    "This typically means geom_sol was loaded from cache but the mask "
                    "geometry was not rebuilt."
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
            scene_cir=Path(str(sol_command.scene_cir) + ".cir"),
            masque_cir=Path(str(sol_command.masque_cir) + ".cir"),
        )

        # Equivalent of the immediate Data creation in initialiser_med(...)
        resul_sol = Data(geom=solene_geometry.geom_sol, type="2D")
        resul_sat = Data(geom=solene_geometry.geom_med, type="3D")

        # Equivalent of self.solEnv = SolEnv(...)
        # Note: this step is only fully faithful once TimeStep and meteo import
        # are also migrated into the new architecture.
        time_step = state.results.get(LEGACY_TIME_STEP)

        sol_env = SolEnv(
            sol_command,
            solene_geometry.geom_sol,
            data=resul_sol,
            timeStep=time_step,
            familles=solene_geometry.extracted_geometry.familles,
        )

        meteo_list = state.results.get(METEO_LIST)
        if meteo_list is not None:
            sol_env.definir_meteo_liste(meteo_list)

        return LegacySoleneEnvironment(
            solene_geometry=solene_geometry,
            export_artifacts=export_artifacts,
            sol_command=sol_command,
            resul_sol=resul_sol,
            resul_sat=resul_sat,
            sol_env=sol_env,
        )

    def _require_solene_geometry(self, state: SimulationState):
        solene_geometry = state.results.get(LEGACY_SOLENE_GEOMETRY)
        if solene_geometry is None:
            raise ValueError(
                "Solene-side geometry is missing; build it before creating the Solene environment."
            )
        return solene_geometry