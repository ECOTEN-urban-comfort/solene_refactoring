
from dataclasses import dataclass
from pathlib import Path

from domain.workspace import RuntimePaths

@dataclass(frozen=True)
class SimulationSettings:

    """
    This dataclass is the first typed replacement for the startup variables that
    were previously extracted directly inside `Simulation.py`.

    In the original code, `Simulation.py` reads `sim_settings.xml` and assigns individual values.

    Those values were then kept as loose local/global variables and passed further
    into the simulation startup logic.

    These values represent the loaded simulation input, not working mutable state.
    Once loaded from XML, they should not be silently modified by downstream code.
    Therefore, we use a frozen dataclass (frozen=True) to ensure immutability.
    """

    begin_day: int
    begin_month: int
    begin_hour: int
    end_day: int
    end_month: int
    end_hour: int
    latitude: float
    longitude: float
    surface_model: str
    air_model: str
    ts_coupl: int
    iter_init: int
    iter_foll: int
    cores_used: int

    @property
    def interval(self) -> list[int]:

        """
        In the old implementation, `Simulation.py` builds a list called `interval`
        after reading the XML settings.

        That structure is then used downstream by the original execution flow,
        especially in time-step initialization / simulation scheduling logic.
        """

        return [
            self.begin_day,
            self.begin_month,
            self.begin_hour,
            0,
            self.end_day,
            self.end_month,
            self.end_hour,
            0,
            1,
            0,
        ]

@dataclass(frozen=True)
class InputFiles:
    med_file: Path
    meteo_file: Path
    sim_settings_file: Path
    famille_file: Path
    materiau_file: Path

@dataclass
class SimulationBootstrap:
    settings: SimulationSettings
    input_files: InputFiles
    paths: RuntimePaths