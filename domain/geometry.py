# Domain models for intermediate and extracted geometry data.
#
# This module defines the typed objects that represent the main outputs of the
# current geometry workflow stages:
# - PreparedGeometryInputs: the staged technical input package used by geometry adapters,
# - LegacyExtractedGeometry: the first wrapped result returned by the legacy
#   MED/family/material extraction pipeline.
#
# These models let the refactored code describe geometry workflow milestones
# explicitly, without forcing the application layer to depend directly on raw
# filesystem conventions or unstructured legacy return values.

from dataclasses import dataclass
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class PreparedGeometryInputs:
    """
    Result of the first technical geometry preparation step.

    This is still not parsed geometry. It only represents the staged technical
    input package that downstream extraction logic will consume.
    """
    stage_dir: Path
    staged_med_file: Path
    staged_famille_file: Path
    staged_materiau_file: Path
    sauvegarde_geom_med: Path
    sauvegarde_geom_sol: Path
    initialization_mode: str = "med"


@dataclass(frozen=True)
class LegacyExtractedGeometry:
    """
    Result of the first true legacy extraction step.

    This wraps legacy objects returned by the old MED/family/material pipeline.
    """
    prepared_inputs: PreparedGeometryInputs
    geom_med: Any
    familles: Any
    n_points: int
    n_triangles: int
    n_tetras: int