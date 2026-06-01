from dataclasses import dataclass
from pathlib import Path
from typing import Any

from domain.geometry import LegacySoleneGeometry


@dataclass(frozen=True)
class SoleneExportArtifacts:
    """
    Result of exporting Solene geometry files needed by the legacy Solene runtime.

    This wraps the two .cir files produced by the equivalent of
    `exporter_geom_solene()`.
    """
    scene_cir: Path
    masque_cir: Path


@dataclass(frozen=True)
class LegacySoleneEnvironment:
    """
    Result of creating the first Solene runtime environment in the refactored code.

    What this represents:
    ---------------------
    - exported Solene geometry artifacts (.cir files),
    - the Solene-side geometry bundle,
    - legacy Data objects for 2D/3D result containers,
    - the instantiated legacy SolEnv object,
    - the instantiated legacy SolCommand object.

    Why this wrapper exists:
    ------------------------
    At this stage we still use legacy objects directly, but we want the application
    layer to hold one typed result rather than a loose set of unrelated variables.
    """
    solene_geometry: LegacySoleneGeometry
    export_artifacts: SoleneExportArtifacts
    sol_command: Any
    resul_sol: Any
    resul_sat: Any
    sol_env: Any