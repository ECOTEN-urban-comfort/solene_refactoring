from pathlib import Path

from application.ports.solene_gateway import SoleneGateway
from domain.artifact_keys import LEGACY_SOLENE_GEOMETRY, LEGACY_EXTRACTED_GEOMETRY
from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment
from domain.geometry import SoleneGeometryArtifacts

from infrastructure.solene.sol_command import SolCommand
from infrastructure.saturne.sat_command import SatCommand
from infrastructure.solene.data import Data
from infrastructure.solene.sol_env import SolEnv
from infrastructure.solene.timeStep import TimeStep
import infrastructure.solene.meteo as meteo_obj
from infrastructure.solene.hdfFile import MedFile
from infrastructure.solene.famille import importer_familles_xml, Familles

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

    def extract_familles(
        self,
        state: SimulationState,
    ) -> Familles:
        """
        Unified technical geometry gateway.

        This adapter currently owns:
            - deterministic staging of geometry-related inputs,
            - legacy `.cpl` MED geometry cache access,
            - first legacy MED/family/material extraction.
        """
        prepared = self._require_prepared_inputs(state)

        # Step 1: extract MED geometry using the old reader.
        med_file = MedFile(str(prepared.staged_med_file))
        geom_med = med_file.extraire_geom()

        # Step 2: load family definitions from XML.
        bootstrap = state.require_bootstrap_definition()
        surface_model = bootstrap.settings.surface_model
        familles = importer_familles_xml(str(prepared.staged_famille_file), surface_model)

        # Step 3: enrich family library with materials.
        familles.importer_materiaux_from_xml(str(prepared.staged_materiau_file))

        # Step 4: bind family numbers extracted from MED.
        familles.attribuer_num_familles(geom_med.familles)

        return familles

    def create_environment(self, state: SimulationState) -> LegacySoleneEnvironment:
        geometry = self._require_solene_geometry(state)
        bootstrap = state.require_bootstrap_definition()

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
        )

        time_step, meteo_list, meteo = self._build_time_step_and_meteo(
            bootstrap=bootstrap,
            sol_command=sol_command,
        )

        geom_sol = self._load_geom(geometry.geom_sol_cpl, "geom_sol")
        geom_med = self._load_geom(geometry.geom_med_cpl, "geom_med")

        resul_sol = Data(geom=geom_sol, type="2D")
        resul_sat = Data(geom=geom_med, type="3D")

        sol_env = SolEnv(
            sol_command,
            geom_sol,
            bootstrap.settings.surface_model,
            data=resul_sol,
            timeStep=time_step,
            familles=state.results[LEGACY_EXTRACTED_GEOMETRY].familles,
        )

        if meteo_list:
            sol_env.definir_meteo_liste(meteo_list)

        return LegacySoleneEnvironment(
            solene_geometry=geometry,
            sol_command=sol_command,
            sat_command=sat_command,
            familles=state.results[LEGACY_EXTRACTED_GEOMETRY].familles,
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

        return time_step, meteo_list, meteo

    def _require_solene_geometry(self, state: SimulationState) -> SoleneGeometryArtifacts:
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