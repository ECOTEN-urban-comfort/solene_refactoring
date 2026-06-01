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

from typing import Any, Protocol

from domain.geometry import LegacyExtractedGeometry, LegacySoleneGeometry, PreparedGeometryInputs
from domain.simulation_state import SimulationState


class GeometryGateway(Protocol):
    """
    Application-facing technical contract for geometry preprocessing.

    At the current refactoring stage, this port covers one coherent geometry
    workflow:
        - staging geometry-related inputs,
        - loading cached MED geometry if it exists,
        - saving freshly extracted MED geometry,
        - executing the first legacy MED/family/material extraction step.
    """

    def prepare_inputs(self, state: SimulationState) -> PreparedGeometryInputs:
        """
        Prepare staged geometry-related inputs for technical processing.
        """

    def extract_geometry(
        self,
        state: SimulationState,
    ) -> LegacyExtractedGeometry:
        """
        Execute the first legacy MED/family/material extraction step.
        """

    def has_saved_med_geometry(self, prepared_inputs: PreparedGeometryInputs) -> bool:
        """
        Return True if the cached volumetric MED geometry already exists.
        """

    def load_saved_med_geometry(self, prepared_inputs: PreparedGeometryInputs) -> Any:
        """
        Load the cached volumetric MED geometry from persistence.

        Returns:
        - Any: the legacy geometry object (`Geom`) restored from `.cpl`.
        """

    def save_med_geometry(
        self,
        prepared_inputs: PreparedGeometryInputs,
        geom_med: Any,
    ) -> None:
        """
        Persist the current volumetric MED geometry to the cache.
        """

    def build_solene_geometry(
        self,
        state: SimulationState,
    ) -> LegacySoleneGeometry:
        """
        Execute the Solene-side geometry branch.

        This method is the refactored equivalent of:
            - checking geom_sol.cpl cache,
            - loading cached Solene geometry if present,
            - otherwise reconstructing geom_med,
            - deriving geom_sol / geom_sol_masque,
            - saving updated geometry caches.

        Export of `.cir` files is intentionally deferred until the Solene command
        path model exists in the refactored code.
        """