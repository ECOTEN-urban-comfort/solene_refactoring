from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import platform


@dataclass(frozen=True)
class CommonCExternalTools:
    geode_ciel: Path
    angl_solid: Path
    facform: Path
    facform_ciel: Path
    masques_sol_lum: Path
    masques_ciel_lum: Path
    radiosite: Path
    rot: Path
    val_op_val: Path
    surf_cont: Path
    luminance_ciel_temps: Path

    temp_dir: Path
    solene_home: Path

    simulation_ts_energie_bat_azam: Path
    simulation_ts_energie_bat_bb5: Path
    simulation_ts_energie_bat_mixture: Path


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

        temp_dir=Path("/home/sol_user/solene_refactoring/common_c_tools/temp/"),
        solene_home=Path("/home/sol_user/solene_refactoring/common_c_tools/exe/"),

        simulation_ts_energie_bat_azam=_require_file(platform_dir / f"simulation_Ts_EnergieBat_azam.exe"),
        simulation_ts_energie_bat_bb5=_require_file(platform_dir / f"simulation_Ts_EnergieBat_bb5.exe"),
        simulation_ts_energie_bat_mixture=_require_file(platform_dir / f"simulation_Ts_EnergieBat_mixture.exe"),

        #simulation_ts_energie_bat_azam=_require_file(platform_dir / "simulation_Ts_EnergieBat_azam"),
        #simulation_ts_energie_bat_bb5=_require_file(platform_dir / "simulation_Ts_EnergieBat_bb5"),
        #simulation_ts_energie_bat_mixture=_require_file(platform_dir / "simulation_Ts_EnergieBat_mixture"),
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

        temp_dir=Path("/home/sol_user/solene_refactoring/common_c_tools/temp/"),
        solene_home=Path("/home/sol_user/solene_refactoring/common_c_tools/exe/"),

        simulation_ts_energie_bat_azam=_require_file(exe_dir / f"simulation_Ts_EnergieBat_azam{suffix}"),
        simulation_ts_energie_bat_bb5=_require_file(exe_dir / f"simulation_Ts_EnergieBat_bb5{suffix}"),
        simulation_ts_energie_bat_mixture=_require_file(exe_dir / f"simulation_Ts_EnergieBat_mixture{suffix}"),
    )