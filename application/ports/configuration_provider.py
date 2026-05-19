# Application-level contract for obtaining simulation configuration.
# This module defines the interface that the application layer depends on when it needs configuration data 
# for a simulation run. It should not know whether the configuration comes from XML, YAML, environment variables, 
# CLI arguments, or any other source. Its purpose is to decouple orchestration logic from configuration storage 
# and parsing details.

# Its responsibility is therefore:
# - define what configuration the application needs
# - expose it through an abstract interface
# - prevent simulation_service and related application code from depending on file format or infrastructure details