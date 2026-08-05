from typing import Protocol

from infrastructure.solene.famille import Familles
from infrastructure.saturne.sat_command import SatCommand
from domain.simulation_definition import SimulationBootstrap


class SaturneGateway(Protocol):
    def initialize(
        self,
        sat_command: SatCommand,
        families: Familles,
        n_proc_saturne: int,
        n_triangles: int,
        bootstrap: SimulationBootstrap,
    ) -> None:
        ...

    def run_initial(
        self,
        sat_command: SatCommand,
        meteo: dict[str, float],
        iterations: int,
        terminal: bool,
    ):
        ...

    def run_restart(
        self,
        sat_command: SatCommand,
        meteo: dict[str, float],
        additional_iterations: int,
        terminal: bool,
    ):
        ...