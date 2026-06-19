from dataclasses import dataclass
from typing import Literal


AirHcMode = Literal["scalar", "per_triangle"]


@dataclass(frozen=True)
class AirModelDefinition:
    name: str
    hc_mode: AirHcMode
    requires_previous_surface_temperature: bool
    default_hc_init: float | None = None
    roughness_length_m: float = 0.4
    reference_height_m: float = 10.0

ASHRAE_AIR_MODEL = AirModelDefinition(
    name="ashrae",
    hc_mode="scalar",
    requires_previous_surface_temperature=False,
)

DENBY_AIR_MODEL = AirModelDefinition(
    name="denby",
    hc_mode="scalar",
    requires_previous_surface_temperature=False,
)

NUSSELT_AIR_MODEL = AirModelDefinition(
    name="nusselt",
    hc_mode="per_triangle",
    requires_previous_surface_temperature=True,
)

VEHRENCAMP_AIR_MODEL = AirModelDefinition(
    name="vehrencamp",
    hc_mode="per_triangle",
    requires_previous_surface_temperature=True,
)