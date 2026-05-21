# Application-facing technical contract for geometry preprocessing.
#
# This module defines the port that the application layer depends on when it
# needs technical geometry work to be performed.
#
# The gateway currently exposes two capabilities:
# - prepare staged geometry inputs for downstream processing,
# - execute the first legacy MED/family/material extraction step.
#
# Its purpose is to let the application layer say "prepare geometry" or
# "extract geometry" without knowing:
# - how files are staged,
# - where temporary working copies are stored,
# - which legacy modules are used,
# - or how MED/family/material parsing is technically implemented.

from typing import Protocol

from domain.geometry import LegacyExtractedGeometry, PreparedGeometryInputs
from domain.simulation_state import SimulationState


class GeometryGateway(Protocol):
    """
    Application-facing technical contract for geometry preprocessing.

    Why one port:
    -------------
    At this stage, geometry staging and first legacy extraction are still part
    of one coherent geometry workflow, so they share one technical boundary.

    Why multiple methods:
    ---------------------
    The workflow still contains distinct sub-capabilities:
        - preparing technical inputs
        - extracting legacy geometry from them
    """

    def prepare_inputs(self, state: SimulationState) -> PreparedGeometryInputs:
        """
        Prepare staged geometry-related inputs for technical processing.
        """

    def extract_legacy_geometry(
        self,
        state: SimulationState,
    ) -> LegacyExtractedGeometry:
        """
        Execute the first legacy MED/family/material extraction step.
        """