# 1) Parses command line arguments 
# 2) composes dependencies
# 3) creates request object
# 4) calls simulation service..

import argparse
from pathlib import Path

from infrastructure.config.xml_configuration_provider import XmlConfigurationProvider


def main() -> int:
    """
    At this first step, the CLI is intentionally minimal.

    It does *not* run the simulation yet.
    It only proves that the refactored bootstrap path works.

    In the original code, startup, configuration loading, filesystem discovery,
    and solver execution are tightly coupled in one flow.

    Here we deliberately stop after configuration/bootstrap loading so that the
    first migrated slice can be validated independently.

    This means the very first executable milestone of the refactoring is:

        "Can the new architecture correctly load the same startup definition
         that the old script would have used?"

    Only after that is stable should we move to runtime initialization,
    directory creation, solver adapters, and coupling.
    """
    parser = argparse.ArgumentParser(
        description="Bootstrap a SOLENE simulation case."
    )
    parser.add_argument(
        "sim_folder",
        type=Path,
        help="Folder containing .med, meteo .csv and sim_settings.xml",
    )
    args = parser.parse_args()

    provider = XmlConfigurationProvider()
    bootstrap = provider.load(args.sim_folder)

    # The prints below are intentionally diagnostic rather than user-polished.
    # Their purpose is to help developers compare the new bootstrap behavior with
    # the old startup values from `Simulation.py`.
    print("Simulation bootstrap loaded successfully")
    print(f"Case: {bootstrap.paths.case_name}")
    print(f"MED file: {bootstrap.input_files.med_file}")
    print(f"Meteo file: {bootstrap.input_files.meteo_file}")
    print(f"sim_settings.xml: {bootstrap.input_files.sim_settings_file}")
    print(f"famille.xml: {bootstrap.input_files.famille_file}")
    print(f"materiau.xml: {bootstrap.input_files.materiau_file}")
    print(f"Surface model: {bootstrap.settings.surface_model}")
    print(f"Air model: {bootstrap.settings.air_model}")
    print(f"Interval: {bootstrap.settings.interval}")
    print(f"Cores: {bootstrap.settings.cores_used}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())