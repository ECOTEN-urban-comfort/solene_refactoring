from infrastructure.solene.profiles.surface_model_profile import SurfaceModelProfile
from infrastructure.solene.profiles.azam import AZAM_PROFILE
from infrastructure.solene.profiles.bb5 import BB5_PROFILE
from infrastructure.solene.profiles.mixture import MIXTURE_PROFILE

SURFACE_MODEL_PROFILES: dict[str, SurfaceModelProfile] = {
    AZAM_PROFILE.name: AZAM_PROFILE,
    BB5_PROFILE.name: BB5_PROFILE,
    MIXTURE_PROFILE.name: MIXTURE_PROFILE,
}


def get_surface_model_profile(surface_model: str) -> SurfaceModelProfile:
    key = surface_model.strip().lower()

    try:
        return SURFACE_MODEL_PROFILES[key]
    except KeyError as exc:
        raise ValueError(
            f"Unsupported surface_model '{surface_model}'. "
            f"Supported values: {sorted(SURFACE_MODEL_PROFILES.keys())}"
        ) from exc