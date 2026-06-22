from dataclasses import dataclass

from domain.air_model_definition import AirModelDefinition
from infrastructure.solene.air_models.runner import (
    AirModelRunConfig,
    AirModelRunner,
)


@dataclass
class AirModelService:
    runner: AirModelRunner

    def run(
        self,
        sim,
        air_model: AirModelDefinition,
        *,
        ts_coupl: int,
        hc_init: float,
        z_ref_m: float = 10.0,
        z_target_m: float = 1.5,
        z0_m: float = 0.4,
    ) -> None:
        config = AirModelRunConfig(
            ts_coupl=ts_coupl,
            hc_init=hc_init,
            z_ref_m=z_ref_m,
            z_target_m=z_target_m,
            z0_m=z0_m,
        )

        self.runner.run(
            sim=sim,
            air_model=air_model,
            config=config,
        )

    def run_from_bootstrap(self, sim, bootstrap) -> None:
        self.run(
            sim,
            bootstrap.air_model,
            ts_coupl=bootstrap.settings.ts_coupl,
            hc_init=bootstrap.settings.hc_init,
        )