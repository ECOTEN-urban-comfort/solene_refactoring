from pathlib import Path

from application.ports.saturne_gateway import SaturneGateway
from infrastructure.saturne import sat_command
from infrastructure.solene.famille import Familles
from infrastructure.solene.utils import ecrire_fichier
from infrastructure.saturne.sat_command import SatCommand
from domain.simulation_definition import SimulationBootstrap


class LegacySaturneGateway(SaturneGateway):
    """
    Transfers legacy SimulationCouplee.initialiser_saturne() into the new
    architecture.
    """

    def initialize(
        self,
        sat_command: SatCommand,
        families: Familles,
        n_proc_saturne: int,
        n_triangles: int,
        bootstrap: SimulationBootstrap,
    ) -> None:

        # equivalent of self.SatCommand.copier_geometrie(self.chemin_geometrie)
        sat_command.copier_geometrie(str(bootstrap.input_files.med_file))

        # equivalent of self.SatCommand.recuperer_listes_from_famille(self.familles)
        sat_command.recuperer_listes_from_famille(families)

        # equivalent of self.SatCommand.ecrire_subroutines()
        sat_command.ecrire_subroutines()

        # equivalent of self.SatCommand.creer_param_xml()
        sat_command.creer_param_xml()

        # equivalent of change_num_processers
        if n_proc_saturne > 1:
            sat_command.change_num_processers(n_proc_saturne)

        # equivalent of create_constant_data(... Fsensible ...)
        self._create_constant_data(
            n_proc_saturne,
            n_triangles,
            sat_command.nom_dat["Fsensible"],
            0.0,
        )

        # equivalent of create_constant_data(... Flatent ...)
        self._create_constant_data(
            n_proc_saturne,
            n_triangles,
            sat_command.nom_dat["Flatent"],
            0.0,
        )

        # equivalent of ecrire_fichier(.../z0, '10.0')
        z0_path = Path(sat_command.chemins["data"]) / "z0"
        ecrire_fichier(str(z0_path), "10.0")

    def run_initial(
        self,
        *,
        sat_command: SatCommand,
        meteo: dict,
        iterations: int,
    ) -> None:
        sat_command.change_num_iteration(iterations)
        sat_command.definir_meteo(meteo)
        sat_command.launch_simulation(terminal=False)
        sat_command.follow_simulation()

    def run_restart(
        self,
        sat_command: SatCommand,
        meteo: dict,
        additional_iterations: int,
        terminal: bool = True,
    ) -> None:
        sat_command.definir_meteo(meteo)

        restart_result = sat_command.definir_restart()
        if restart_result != 0:
            raise RuntimeError(
                "Code_Saturne restart checkpoint was not found."
            )

        sat_command.ajouter_iterations(
            additional_iterations
        )

        sat_command.launch_simulation(
            terminal=terminal,
        )
        sat_command.follow_simulation()

    def _create_constant_data(
        self,
        n_proc_saturne: int,
        n_triangles: int,
        nom_dat: str,
        value: float,
    ) -> None:
        n_triangles
        text = (f"{value}\n") * n_triangles

        if n_proc_saturne == 1:
            ecrire_fichier(nom_dat, text)
        else:
            for i in range(n_proc_saturne):
                ecrire_fichier(f"{nom_dat}_{i}", text)