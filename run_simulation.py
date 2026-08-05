# Temporary CLI composition root for the refactored simulation startup flow.
#
# This module is responsible for:
# - parsing command-line arguments,
# - composing the currently migrated dependencies,
# - loading the simulation bootstrap from the case folder,
# - initializing the runtime session,
# - running the first geometry preparation and legacy extraction steps,
# - printing diagnostic output for developers.
#
# At the current refactoring stage, this script is not yet the full production
# execution entry point. Its main purpose is to validate that the migrated
# startup pipeline works end to end before solver execution and coupling logic
# are moved into the new architecture.

from application.geometry_service import GeometryService
from application.runtime_session_service import RuntimeSessionService
from infrastructure.config.xml_configuration_provider import XmlConfigurationProvider
from application.geometry_service import GeometryService
from infrastructure.geometry.legacy_geometry_gateway import LegacyGeometryGateway
from application.solene_service import SoleneService
from infrastructure.solene.legacy_solene_gateway import LegacySoleneGateway
from application.air_model_service import AirModelService
from infrastructure.solene.air_models.runner import AirModelRunner
from application.saturne_service import SaturneService
from infrastructure.saturne.legacy_saturne_gateway import LegacySaturneGateway


def main() -> int:
    """
    parser = argparse.ArgumentParser(
        description="Bootstrap and initialize a SOLENE simulation case."
    )
    parser.add_argument(
        "sim_folder",
        type=Path,
        help="Folder containing MED, meteo CSV, sim_settings.xml, famille.xml, and materiau.xml",
    )
    args = parser.parse_args()
    """
    # Step 1: load startup definition from the case folder.
    configuration_provider = XmlConfigurationProvider()
    bootstrap = configuration_provider.load("/home/sol_user/sim_files")

    # Step 2: initialize runtime workspace and create the initial SimulationState.
    runtime_session_service = RuntimeSessionService()
    state = runtime_session_service.initialize(bootstrap)

    # Step 3: initialize geometry workflow using one merged technical gateway.
    geometry_gateway = LegacyGeometryGateway()
    geometry_service = GeometryService(geometry_gateway)

    # 3a) prepare staged geometry inputs
    state = geometry_service.initialize(state)

    # 3b) execute the first true legacy MED/family/material extraction step
    state = geometry_service.extract_families(state)

    # 3c) execute the Solene-side geometry branch
    state = geometry_service.build_solene_geometry(state)

    solene_gateway = LegacySoleneGateway()
    solene_service = SoleneService(solene_gateway)

    # after Solene-side geometry exists
    state = solene_service.create_environment(state)

    state = solene_service.prepare_shared_runtime(state)
    
    saturne_service = SaturneService(gateway=LegacySaturneGateway())
    state = saturne_service.initialize(state)

    air_model_service = AirModelService(runner=AirModelRunner())
    state = air_model_service.run(state)

    state = saturne_service.run_initial(state)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())