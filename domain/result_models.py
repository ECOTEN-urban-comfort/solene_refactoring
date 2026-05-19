# Not an application service, but a domain layer for result representation. For instance it makes sure the app
# doesn't work with unstructured dicts, but with well defined objects. It defines:
# A) the system known result types
# B) how the results are represented in the system (e.g. as a dataclass)
# C) result properties
# D) result optionality

# i.e. ResultArtifact, ScalarResult, FieldResult, SimulationResult