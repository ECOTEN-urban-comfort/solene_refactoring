from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from domain.geometry import SoleneGeometryArtifacts
from infrastructure.solene.timeStep import TimeStep
from infrastructure.solene.sol_command import SolCommand
from infrastructure.solene.sol_env import SolEnv
from infrastructure.saturne.sat_command import SatCommand

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
    solene_geometry: SoleneGeometryArtifacts
    sol_command: SolCommand
    sat_command: SatCommand
    familles: Any
    resul_sol: Any
    resul_sat: Any
    sol_env: SolEnv
    time_step: TimeStep | None = None
    meteo_list: list[dict[str, Any]] = field(default_factory=list)
    meteo: dict[str, dict[str, Any]] = field(default_factory=dict)
    n_proc_saturne: int = 1