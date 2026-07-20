from typing import Protocol, Tuple

from domain.simulation_state import SimulationState
from infrastructure.solene.timeStep import TimeStep
from infrastructure.solene.sol_command import SolCommand
from infrastructure.solene.sol_env import SolEnv
from infrastructure.saturne.sat_command import SatCommand


class SoleneGateway(Protocol):
    """
    Application-facing contract for Solene runtime setup.

    This port currently covers:
        - environment creation after Solene geometry exists,
        - shared runtime preparation common to all air models.
    """

    def create_environment(self, state: SimulationState) -> Tuple[SolCommand, SatCommand, SolEnv, TimeStep, dict]:
        """
        Create the first Solene runtime environment for the current run.
        """

    def prepare_shared_runtime(self, sol_command: SolCommand, sol_env: SolEnv) -> None:
        """
        Execute the shared Solene runtime preparation that is common for all
        air models and happens after environment creation.
        """