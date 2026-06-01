# Shared symbolic names for artifacts, artifact categories, and result keys.
#
# This module centralizes the string identifiers used across the runtime state
# and application services so they do not have to be duplicated inline.
#
# It currently provides:
# - artifact keys for known input and staged files,
# - artifact type names,
# - result keys used in SimulationState.results.
#
# Its purpose is to prevent inconsistent string usage across modules and to make
# renaming or extending artifact vocabulary safer and easier.

GEOMETRY_MED = "geometry_med"
METEO_CSV = "meteo_csv"
SIM_SETTINGS_XML = "sim_settings_xml"
FAMILLE_XML = "famille_xml"
MATERIAU_XML = "materiau_xml"

STAGED_GEOMETRY_MED = "staged_geometry_med"
STAGED_FAMILLE_XML = "staged_famille_xml"
STAGED_MATERIAU_XML = "staged_materiau_xml"

INPUT_ARTIFACT = "input"
PREPARED_INPUT_ARTIFACT = "prepared_input"
GENERATED_INPUT_ARTIFACT = "generated_input"
OUTPUT_ARTIFACT = "output"
RESULT_ARTIFACT = "result"

PREPARED_GEOMETRY_INPUTS = "prepared_geometry_inputs"
LEGACY_EXTRACTED_GEOMETRY = "legacy_extracted_geometry"
LEGACY_SOLENE_GEOMETRY = "legacy_solene_geometry"
LEGACY_TIME_STEP = "legacy_time_step"
METEO_LIST = "meteo_list"