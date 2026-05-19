# Domain representation of the current simulation state during its life cycle.
# This module describes:
# A) in which phase of the simulation life cycle we are (e.g. pre-processing, running, post-processing)
# B) what are the inputs
# C) which steps already passed
# D) which artifacts exist at the moment
# E) which is valid and which is not

# it is an object passing amongst the application services:
# - geometry_service.py updates geometry_initialized=True
# - coupling_service.py updates coupling_initialized=True

# what belongs to the object:
# - run id
# - workspace
# - configuration fingerprint
# - state of individual steps
# - reference to artifacts
# - reference to results