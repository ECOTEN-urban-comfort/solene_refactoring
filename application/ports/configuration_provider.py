# Application-level contract for obtaining simulation configuration.
#
# This module defines the interface that the application layer depends on when
# it needs a simulation case definition.
#
# It deliberately hides the source of configuration data, so orchestration code
# does not need to know whether the configuration comes from:
# - XML files,
# - YAML files,
# - environment variables,
# - CLI arguments,
# - test fixtures,
# - or any other source.
#
# The output of this contract is a SimulationBootstrap: a fully assembled startup
# definition containing typed simulation settings, discovered input files, and
# derived runtime paths.

from pathlib import Path
from typing import Protocol

from domain.simulation_definition import SimulationBootstrap


class ConfigurationProvider(Protocol):
    """
    In the original codebase, there is no explicit abstraction for configuration
    loading. The startup script simply knows how to:
        - inspect the folder
        - read XML
        - locate input files
        - assemble the initial values

    That is convenient in a script, but problematic in a layered architecture.

    We want the application layer to depend only on the *fact* that configuration
    can be loaded, not on *how* it is loaded.

    In other words:
        - the application should not know whether settings come from XML,
          YAML, environment variables, a database, or a test fixture
        - it should only know that some provider can produce a
          `SimulationBootstrap`

    Configuration loading is one of the easiest responsibilities to separate out
    from the legacy script. It is also foundational, because almost every other
    future service depends on it.

    This is the abstract replacement for the implicit startup logic that lives
    mostly in `Simulation.py` and partially relies on XML helper behavior similar
    to what exists in `xmlFile.py`.
    """

    def load(self, sim_folder: Path) -> SimulationBootstrap:
        """
        Load the simulation case definition from the given simulation folder.

        Expected output of this first-step provider:
            - discovered MED filename
            - discovered meteo CSV filename
            - parsed XML settings
            - derived canonical runtime paths
        """