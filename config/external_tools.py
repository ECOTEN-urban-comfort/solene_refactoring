from dataclasses import dataclass
from pathlib import Path
from shutil import which


def _resolve_tool(name: str, configured_path: str | None = None) -> Path:
    """
    Resolve an external executable either from an explicit configured path
    or from PATH.
    """
    if configured_path:
        path = Path(configured_path)
        if not path.is_file():
            raise FileNotFoundError(
                f"Configured executable for '{name}' was not found: {path}"
            )
        return path

    found = which(name)
    if found:
        return Path(found)

    raise FileNotFoundError(
        f"Executable '{name}' was not found in PATH and no explicit path was configured."
    )


@dataclass(frozen=True)
class SoleneExternalTools:
    geode_ciel: Path
    angl_solid: Path
    surf_cont: Path

    luminance_ciel_temps: Path
    luminance_ciel: Path

    masques_sol_lum: Path
    energie_solaire_directe_meteo: Path
    masques_sol_lum_meteo: Path

    masques_ciel_lum: Path
    energie_solaire_diffuse_meteo: Path
    masques_ciel_lum_meteo: Path

    val_op_val: Path

    facform_lum: Path
    facform: Path
    facform_ciel: Path

    radiosite: Path

    simulation_ts_energie_bat: Path
    simulation_ts_energie_bat_veg: Path
    simulation_ts_energie_bat_new: Path

    # Optional transitive dependency used from inside geode_ciel.c
    rot: Path | None = None

    @classmethod
    def resolve(
        cls,
        *,
        geode_ciel: str | None = None,
        angl_solid: str | None = None,
        surf_cont: str | None = None,
        luminance_ciel_temps: str | None = None,
        luminance_ciel: str | None = None,
        masques_sol_lum: str | None = None,
        energie_solaire_directe_meteo: str | None = None,
        masques_sol_lum_meteo: str | None = None,
        masques_ciel_lum: str | None = None,
        energie_solaire_diffuse_meteo: str | None = None,
        masques_ciel_lum_meteo: str | None = None,
        val_op_val: str | None = None,
        facform_lum: str | None = None,
        facform: str | None = None,
        facform_ciel: str | None = None,
        radiosite: str | None = None,
        simulation_ts_energie_bat: str | None = None,
        simulation_ts_energie_bat_veg: str | None = None,
        simulation_ts_energie_bat_new: str | None = None,
        rot: str | None = None,
    ) -> "SoleneExternalTools":
        return cls(
            geode_ciel=_resolve_tool("geode_ciel", geode_ciel),
            angl_solid=_resolve_tool("angl_solid", angl_solid),
            surf_cont=_resolve_tool("surf_cont", surf_cont),

            luminance_ciel_temps=_resolve_tool(
                "luminance_ciel_temps", luminance_ciel_temps
            ),
            luminance_ciel=_resolve_tool("luminance_ciel", luminance_ciel),

            masques_sol_lum=_resolve_tool("masques_sol_lum", masques_sol_lum),
            energie_solaire_directe_meteo=_resolve_tool(
                "energie_solaire_directe_meteo", energie_solaire_directe_meteo
            ),
            masques_sol_lum_meteo=_resolve_tool(
                "masques_sol_lum_meteo", masques_sol_lum_meteo
            ),

            masques_ciel_lum=_resolve_tool("masques_ciel_lum", masques_ciel_lum),
            energie_solaire_diffuse_meteo=_resolve_tool(
                "energie_solaire_diffuse_meteo", energie_solaire_diffuse_meteo
            ),
            masques_ciel_lum_meteo=_resolve_tool(
                "masques_ciel_lum_meteo", masques_ciel_lum_meteo
            ),

            val_op_val=_resolve_tool("val_op_val", val_op_val),

            facform_lum=_resolve_tool("facform_lum", facform_lum),
            facform=_resolve_tool("facform", facform),
            facform_ciel=_resolve_tool("facform_ciel", facform_ciel),

            radiosite=_resolve_tool("radiosite", radiosite),

            simulation_ts_energie_bat=_resolve_tool(
                "simulation_Ts_EnergieBat_laurent",
                simulation_ts_energie_bat,
            ),
            simulation_ts_energie_bat_veg=_resolve_tool(
                "simulation_Ts_EnergieBat_VEG",
                simulation_ts_energie_bat_veg,
            ),
            simulation_ts_energie_bat_new=_resolve_tool(
                "simulation_Ts_EnergieBat.exe",
                simulation_ts_energie_bat_new,
            ),

            rot=Path(rot) if rot else None,
        )