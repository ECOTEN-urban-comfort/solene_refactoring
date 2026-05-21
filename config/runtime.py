# Runtime path builder for a single simulation case.
#
# This module centralizes the derivation of the canonical workspace layout used
# during execution.
#
# Given the simulation input folder, it computes the standard runtime paths for:
# - the case root,
# - sauvegarde,
# - temp,
# - post,
# - simulSol,
# - simulSat.
#
# Its purpose is to extract filesystem-layout logic from the old startup code
# and make the runtime directory structure explicit, deterministic, and reusable
# across services.

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