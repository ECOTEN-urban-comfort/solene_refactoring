from domain.air_model_definition import (
    AirModelDefinition,
    ASHRAE_AIR_MODEL,
    DENBY_AIR_MODEL,
    NUSSELT_AIR_MODEL,
    VEHRENCAMP_AIR_MODEL,
)

_AIR_MODELS: dict[str, AirModelDefinition] = {
    "ashrae": ASHRAE_AIR_MODEL,
    "denby": DENBY_AIR_MODEL,
    "nusselt": NUSSELT_AIR_MODEL,
    "vehrencamp": VEHRENCAMP_AIR_MODEL,
}


def get_air_model_definition(air_model: str) -> AirModelDefinition:
    key = air_model.strip().lower()
    try:
        return _AIR_MODELS[key]
    except KeyError as exc:
        raise ValueError(
            f"Unsupported air_model '{air_model}'. "
            f"Supported values: {sorted(_AIR_MODELS.keys())}"
        ) from exc