from dataclasses import dataclass
from typing import Mapping


@dataclass(frozen=True)
class SurfaceModelProfile:
    """
    Model-specific configuration extracted from legacy SolCommand/SolEnv modules.

    This profile captures the parts of the legacy implementation that vary by
    surface model, while the execution skeleton remains shared in base classes.
    """

    name: str

    # solEnv.py differences
    meteo_sol_defaults: Mapping[str, float]

    # solCommand.py differences
    carac_face: tuple[str, ...]
    carac_triangle: tuple[str, ...]
    variables_clo: tuple[str, ...]
    variables_glo: tuple[str, ...]
    variables_transient: tuple[str, ...]
    variables_comfort: tuple[str, ...]

    # default runtime parameters
    default_time_step_s: int

    # feature flags controlling optional branches in SolEnv
    enable_water_descriptors: bool
    enable_irrigation_descriptors: bool
    enable_water_flux_outputs: bool
    enable_extra_transient_nodes: bool
    enable_tp_outputs: bool