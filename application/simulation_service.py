# Orchestration of the simulation, taking care of the overall flow of the simulation, operation order, state, 
# dependencies and transitions between individual steps.
# 1) Takes simulation service request from cli/run_simulation.py, which includes the settings for the simulation.
# 2) Initializes geometry and materials - MED import, familles, etc.
# 3) Initializes time and weather - time step, weather data, etc.
# 4) Initializes the simulation loop, i.e. individual models.
# 5) VTU and CFD postprocessing export.