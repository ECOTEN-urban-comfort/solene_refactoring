# Defines the execution context for a single simulation run. It is responsible for runtime-specific values 
# and paths that are only known when the program is launched, such as working directories, temporary folders, 
# generated file locations, run identifiers, and other execution-scoped metadata needed during orchestration.
# More concretely, this module should answer questions like:
# - where this run is executed,
# - where intermediate artifacts are stored,
# - what folders/files belong to this run,
# - what runtime flags are active,
# - what execution-specific paths other services should use.