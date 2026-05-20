from dataclasses import dataclass
from pathlib import Path

@dataclass(frozen=True)
class RuntimePaths:

    """
    This dataclass is the structured replacement for the case-path bootstrap that
    originally happens inside `main.py`, specifically in the initialization logic
    of the simulation/coupling runtime.

    In the original code, the startup procedure derives and stores a set of
    filesystem locations.

    These paths are essential because the legacy code heavily relies on a
    convention-based directory structure. The original logic creates or refers to
    these folders directly while bootstrapping the coupled computation.

    In the old code, these paths are computed procedurally and then kept as
    attributes on runtime objects or implicit local variables. That makes it hard
    to see which filesystem structure the rest of the application depends on.

    By introducing `RuntimePaths`, we make the runtime folder layout explicit and
    transferable as a single object.

    This object only *describes* the paths. It does not create directories yet.
    """

    sim_folder: Path
    case_name: str
    case_root: Path
    sauvegarde_dir: Path
    temp_dir: Path
    post_dir: Path
    simul_sol_dir: Path
    simul_sat_dir: Path