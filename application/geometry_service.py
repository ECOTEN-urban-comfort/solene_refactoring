# Orchestrates the geometry preparation for the simulation models.
# 1) Takes the geometry data.
# 2) Check the geometry for consistency and validity, such as checking for duplicate nodes, checking for 
# non-manifold edges, etc.
# 3) Runs preprocessing steps.
# 4) Geometry object creation for the simulation models, such as creating the geometry for the CFD model, 
# the geometry for the daylight model, etc.