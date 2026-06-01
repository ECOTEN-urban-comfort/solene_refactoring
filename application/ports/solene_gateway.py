from typing import Protocol

from domain.simulation_state import SimulationState
from domain.solene import LegacySoleneEnvironment


class SoleneGateway(Protocol):
    """
    Application-facing contract for Solene runtime preparation.

    This port covers the first Solene-specific step after geometry extraction:
        - exporting Solene geometry files,
        - creating result containers,
        - creating the legacy SolEnv object,
        - wiring meteorological input if available.
    """

    def create_environment(self, state: SimulationState) -> LegacySoleneEnvironment:
        """
        Create the first Solene runtime environment for the current run.
        """