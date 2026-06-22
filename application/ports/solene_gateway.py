from typing import Protocol

from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment


class SoleneGateway(Protocol):
    """
    Application-facing contract for Solene runtime setup.

    This port currently covers:
        - environment creation after Solene geometry exists,
        - shared runtime preparation common to all air models.
    """

    def create_environment(self, state: SimulationState) -> LegacySoleneEnvironment:
        """
        Create the first Solene runtime environment for the current run.
        """

    def prepare_shared_runtime(self, environment: LegacySoleneEnvironment, state: SimulationState) -> None:
        """
        Execute the shared Solene runtime preparation that is common for all
        air models and happens after environment creation.
        """