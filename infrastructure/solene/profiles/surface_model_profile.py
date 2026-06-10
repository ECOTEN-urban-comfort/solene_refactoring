from dataclasses import dataclass
from typing import Mapping, Literal


@dataclass(frozen=True)
class SurfaceModelProfile:
    name: str

    meteo_sol_defaults: Mapping[str, float]

    carac_face: tuple[str, ...]
    carac_triangle: tuple[str, ...]
    variables_clo: tuple[str, ...]
    variables_glo: tuple[str, ...]
    variables_transient: tuple[str, ...]
    variables_comfort: tuple[str, ...]

    default_time_step_s: int

    enable_ecl_inc_tot: bool
    enable_tp_outputs: bool
    enable_water_descriptors: bool
    enable_extra_transient_nodes: bool
    enable_irrigation_descriptors: bool
    enable_water_flux_outputs: bool