import math


def wind_speed_at_height(v_ref: float, z_ref: float, z_target: float, z0: float) -> float:
    """
    Convert wind speed from reference height to target height using logarithmic law.
    """
    if z_ref <= z0 or z_target <= z0:
        return max(v_ref, 0.5)

    v = v_ref * math.log(z_target / z0) / math.log(z_ref / z0)
    return max(v, 0.5)


def ashrae_hc(v_air: float) -> float:
    return 3.8 * v_air + 5.7


def denby_hc(v_air: float, z: float = 10.0, z0: float = 0.4) -> float:
    rhoa = 1.2
    cp = 1004.0
    # sem vlož přesný legacy výpočet rT
    r_t = ...
    return rhoa * cp / r_t


def vehrencamp_hc(t_surf: float, t_air: float, v_air: float) -> float:
    t_m = (t_surf + t_air) / 2.0
    a = 1.4
    d = 0.5
    # sem přesný legacy výraz
    return ...


def nusselt_hc(t_surf: float, t_air: float, v_air: float, characteristic_length: float) -> float:
    # sem přesná legacy větev Re / Gr / Nu
    return ...