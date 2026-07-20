from pathlib import Path

from application.ports.saturne_gateway import SaturneGateway
from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment
from infrastructure.solene.utils import ecrire_fichier


class LegacySaturneGateway(SaturneGateway):
    """
    Transfers legacy SimulationCouplee.initialiser_saturne() into the new
    architecture.
    """

    def initialize(
        self,
        environment: LegacySoleneEnvironment,
        state: SimulationState,
    ) -> None:
        bootstrap = state.require_bootstrap_definition()
        sat_command = environment.sat_command
        familles = environment.familles

        # equivalent of self.SatCommand.copier_geometrie(self.chemin_geometrie)
        sat_command.copier_geometrie(str(bootstrap.input_files.med_file))

        # equivalent of self.SatCommand.recuperer_listes_from_famille(self.familles)
        sat_command.recuperer_listes_from_famille(familles)

        # equivalent of self.SatCommand.ecrire_subroutines()
        sat_command.ecrire_subroutines()

        # equivalent of self.SatCommand.creer_param_xml()
        sat_command.creer_param_xml()

        # equivalent of change_num_processers
        if state.families_extractio > 1:
            sat_command.change_num_processers(state.families_extractio)

        # equivalent of create_constant_data(... Fsensible ...)
        self._create_constant_data(
            environment,
            sat_command.nom_dat["Fsensible"],
            0.0,
        )

        # equivalent of create_constant_data(... Flatent ...)
        self._create_constant_data(
            environment,
            sat_command.nom_dat["Flatent"],
            0.0,
        )

        # equivalent of ecrire_fichier(.../z0, '10.0')
        z0_path = Path(sat_command.chemins["data"]) / "z0"
        ecrire_fichier(str(z0_path), "10.0")

    def _create_constant_data(
        self,
        environment: LegacySoleneEnvironment,
        nom_dat: str,
        value: float,
    ) -> None:
        n_triangles = environment.solene_geometry.geom_sol.n_triangles
        text = (f"{value}\n") * n_triangles

        if environment.families_extractio == 1:
            ecrire_fichier(nom_dat, text)
        else:
            for i in range(environment.families_extractio):
                ecrire_fichier(f"{nom_dat}_{i}", text)