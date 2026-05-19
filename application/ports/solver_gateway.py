# A gate to the computational engine. Tells the engine which model to run, and what data to use for the model. 
# Also, it can be used to retrieve the results from the engine after the model has been run.
# Gets returned "Simulation finished/failed - returned these artefacts."
# Application service - simulation_service.py will not have to know the shell commands to run the engine, the
# folder structure, neither the technical details of the engine. It just has to know that it can call the solver 
# gateway, and get the results back. -> simulation_service.py can be then tested without the engine, and the engine 
# can be tested without the simulation service.