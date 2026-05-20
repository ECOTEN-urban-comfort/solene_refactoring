# Defines the execution context for a single simulation run. It is responsible for runtime-specific values 
# and paths that are only known when the program is launched, such as working directories, temporary folders, 
# generated file locations, run identifiers, and other execution-scoped metadata needed during orchestration.
# More concretely, this module should answer questions like:
# - where this run is executed,
# - where intermediate artifacts are stored,
# - what folders/files belong to this run,
# - what runtime flags are active,
# - what execution-specific paths other services should use.

from pathlib import Path

from domain.workspace import RuntimePaths


def build_runtime_paths(sim_folder: Path) -> RuntimePaths:
    """
    This function centralizes the filesystem-layout derivation that originally
    appeared inside the runtime initialization logic in `main.py`.

    In the old code, the case runtime constructs a family of directories.

    Those paths are not incidental; they are part of the execution contract of
    the legacy workflow. Many downstream parts of the system assume that those
    conventional folder names exist.
    """
    sim_folder = sim_folder.resolve()
    case_name = sim_folder.name
    case_root = sim_folder.parent / case_name

    return RuntimePaths(
        sim_folder=sim_folder,
        case_name=case_name,
        case_root=case_root,
        sauvegarde_dir=case_root / "sauvegarde",
        temp_dir=case_root / "temp",
        post_dir=case_root / "post",
        simul_sol_dir=case_root / "simulSol",
        simul_sat_dir=case_root / "simulSat",
    )