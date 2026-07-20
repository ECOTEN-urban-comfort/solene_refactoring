from typing import Protocol

from domain.simulation_state import SimulationState


class SaturneGateway(Protocol):
    def initialize(
        self,
        state: SimulationState,
    ) -> None:
        """
        Prepare Saturne-side runtime files and folders from the already built
        Solene environment and case bootstrap.
        """