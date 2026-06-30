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
    staged_med_file: Path
    staged_famille_file: Path
    staged_materiau_file: Path

    sauvegarde_dir: Path
    simul_sol_dir: Path

@dataclass(frozen=True)
class SoleneGeometryArtifacts:
    geom_med_cpl: Path
    geom_sol_cpl: Path
    scene_cir: Path
    masque_cir: Path