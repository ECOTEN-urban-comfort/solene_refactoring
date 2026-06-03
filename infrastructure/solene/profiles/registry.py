from infrastructure.solene.profiles.azam import AZAM_PROFILE
from infrastructure.solene.profiles.bb5 import BB5_PROFILE
from infrastructure.solene.profiles.mixture import MIXTURE_PROFILE
from infrastructure.solene.profiles.surface_model_profile import SurfaceModelProfile


_PROFILES: dict[str, SurfaceModelProfile] = {
    "azam": AZAM_PROFILE,
    "BB5": BB5_PROFILE,
    "mixture": MIXTURE_PROFILE,
}


def get_surface_model_profile(surface_model: str) -> SurfaceModelProfile:
    try:
        return _PROFILES[surface_model]
    except KeyError as exc:
        raise ValueError(
            f"Unsupported surface_model '{surface_model}'. "
            f"Supported values: {sorted(_PROFILES.keys())}"
        ) from exc