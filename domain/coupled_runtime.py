from dataclasses import dataclass, field
from typing import Any

from infrastructure.solene.data import Data
from infrastructure.solene.geom import Geom
from infrastructure.solene.famille import Familles
from infrastructure.solene.sol_env import SolEnv
from infrastructure.solene.sol_command import SolCommand
from infrastructure.saturne.sat_command import SatCommand
from infrastructure.solene.timeStep import TimeStep


@dataclass
class LegacyCoupledRuntime:
    sol_command: SolCommand
    sat_command: SatCommand

    geom_sol: Geom
    geom_med: Geom
    geom_sol_masque: Geom

    familles: Familles
    sol_env: SolEnv
    time_step: TimeStep

    meteo_list: list[dict[str, Any]] = field(default_factory=list)
    meteo: dict[str, dict[str, Any]] = field(default_factory=dict)

    resul_sol: Data | None = None
    resul_sat: Data | None = None

    dic_val2dat: dict = field(default_factory=dict)
    dic_dat2val: dict = field(default_factory=dict)
    lien_dat_val: dict = field(default_factory=dict)
    lien_val_dat: dict = field(default_factory=dict)

    dic_num_fac: dict = field(default_factory=dict)
    n_proc_saturne: int = 1