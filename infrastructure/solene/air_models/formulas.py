import math
import numpy as np


def wind_speed_at_height(v_ref: float, z_ref: float, z_target: float, z0: float) -> float:
    """
    Legacy logarithmic wind-speed transformation with lower clamp at 0.5 m/s.
    """
    v = v_ref * (math.log(z_target / z0) / math.log(z_ref / z0))
    if v < 0.5:
        return 0.5
    return v


def ashrae_hc(v_air: float) -> float:
    """
    Legacy ASHRAE convection model.
    """
    return 3.8 * v_air + 5.7


def denby_hc(v_air: float, z: float = 10.0, z0: float = 0.4) -> float:
    """
    Legacy Denby convection model.

    Note:
    In the legacy snippet, `h` is computed but the `return h` line is not visible.
    It must return `h`, otherwise the function would always return None.
    """
    rhoa = 1.2
    cp = 1006
    k = 0.4
    r_t = math.log(z / z0) * math.log(10 * z / z0) / (v_air * k**2)
    h = rhoa * cp / r_t
    return h


def vehrencamp_hc(t_surf: float, t_air: float, v_air: float) -> float:
    """
    Legacy Vehrencamp ground convection model.
    """
    t_m = (t_surf + t_air) / 2
    a = 1.4
    d = 0.5
    h = 698.24 * a * (
        0.00144 * (t_m + 273.15) ** 0.3 * v_air**d
        + 0.00097 * abs(t_surf - t_air) ** 0.3
    )
    return h


def nusselt_hc(
    t_surf: float,
    t_air: float,
    v_air: float,
    characteristic_length: float = 1.0,
) -> float:
    """
    Legacy Nusselt-style mixed convection model.
    """
    nu_air = 1.57 * 10.0 ** (-5.0)
    cp_air = 1006.0
    lamb = 0.0262
    pr = 0.708
    L = characteristic_length
    g = 9.81

    re = v_air * L / nu_air
    ba = 1.0 / (t_air + 273.0)
    gr = ba * g * L**3.0 * abs(t_surf - t_air) / nu_air**2.0

    # Legacy code uses np.mean(Gr) even though Gr is scalar.
    gr_mean = np.mean(gr)

    # natural
    if 10 * re**2 < gr_mean:
        # laminar / turbulent branch kept exactly as in legacy
        if gr_mean < 10**9:
            h = (lamb / L) * 0.49 * gr ** (1.0 / 4.0)
        else:
            h = (lamb / L) * 0.13 * gr ** (1.0 / 3.0)

    # forced
    elif re**2 > 10 * gr_mean:
        if gr_mean < 10**9:
            h = (lamb / L) * 0.56 * re ** (1.0 / 2.0)
        else:
            h = (lamb / L) * 0.03 * re ** (4.0 / 5.0)

    # mixed
    else:
        if gr_mean < 10**9:
            h = (lamb / L) * 0.68 * (0.57 * gr ** (3.0 / 4.0) + re ** (3.0 / 2.0)) ** (1.0 / 3.0)
        else:
            h = (lamb / L) * 0.03 * (12.1 * gr + re ** (12.0 / 5.0)) ** (1.0 / 3.0)

    return h