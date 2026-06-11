from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import platform


@dataclass(frozen=True)
class CommonCExternalTools:
    """
    Resolved paths to legacy native executables from common_c.
    """

    angl_solid: Path
    facform: Path
    facform_ciel: Path
    geode_ciel: Path
    luminance_ciel_temps: Path
    masques_ciel_lum: Path
    masques_sol_lum: Path
    radiosite: Path
    rot: Path
    surf_cont: Path
    val_op_val: Path


def _platform_dir_name() -> str:
    system = platform.system().lower()

    if system.startswith("linux"):
        return "linux-x86_64"
    if system.startswith("windows"):
        return "windows-x86_64"

    raise RuntimeError(f"Unsupported platform for common_c binaries: {system}")


def _exe_suffix() -> str:
    return ".exe" if platform.system().lower().startswith("windows") else ""


def _require_file(path: Path) -> Path:
    if not path.is_file():
        raise FileNotFoundError(f"Required executable was not found: {path}")
    return path


def build_tools_from_bin_dir(bin_root: Path) -> CommonCExternalTools:
    """
    Build tool paths from the new normalized structure:

        common_c/
          bin/
            linux-x86_64/
            windows-x86_64/

    Example:
        bin_root = Path("/.../common_c/bin")
    """
    platform_dir = bin_root / _platform_dir_name()
    suffix = _exe_suffix()

    return CommonCExternalTools(
        angl_solid=_require_file(platform_dir / f"angl_solid{suffix}"),
        facform=_require_file(platform_dir / f"facform{suffix}"),
        facform_ciel=_require_file(platform_dir / f"facform_ciel{suffix}"),
        geode_ciel=_require_file(platform_dir / f"geode_ciel{suffix}"),
        luminance_ciel_temps=_require_file(platform_dir / f"luminance_ciel_temps{suffix}"),
        masques_ciel_lum=_require_file(platform_dir / f"masques_ciel_lum{suffix}"),
        masques_sol_lum=_require_file(platform_dir / f"masques_sol_lum{suffix}"),
        radiosite=_require_file(platform_dir / f"radiosite{suffix}"),
        rot=_require_file(platform_dir / f"rot{suffix}"),
        surf_cont=_require_file(platform_dir / f"surf_cont{suffix}"),
        val_op_val=_require_file(platform_dir / f"val_op_val{suffix}"),
    )


def build_tools_from_legacy_exe_dir(exe_dir: Path) -> CommonCExternalTools:
    """
    Transitional helper for the current legacy layout from your screenshot:

        common_c/
          exe/
            angl_solid
            angl_solid.exe
            facform
            ...

    Example:
        exe_dir = Path("/.../common_c/exe")
    """
    suffix = _exe_suffix()

    return CommonCExternalTools(
        angl_solid=_require_file(exe_dir / f"angl_solid{suffix}"),
        facform=_require_file(exe_dir / f"facform{suffix}"),
        facform_ciel=_require_file(exe_dir / f"facform_ciel{suffix}"),
        geode_ciel=_require_file(exe_dir / f"geode_ciel{suffix}"),
        luminance_ciel_temps=_require_file(exe_dir / f"luminance_ciel_temps{suffix}"),
        masques_ciel_lum=_require_file(exe_dir / f"masques_ciel_lum{suffix}"),
        masques_sol_lum=_require_file(exe_dir / f"masques_sol_lum{suffix}"),
        radiosite=_require_file(exe_dir / f"radiosite{suffix}"),
        rot=_require_file(exe_dir / f"rot{suffix}"),
        surf_cont=_require_file(exe_dir / f"surf_cont{suffix}"),
        val_op_val=_require_file(exe_dir / f"val_op_val{suffix}"),
    )