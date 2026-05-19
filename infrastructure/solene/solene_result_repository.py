# Something like a Solene files API. Works on top of path_resolver.py, result_locator.py, output_reader.py and 
# value_parser.py. Runs requests like:
# - get the albedo value from the .val file
# - get the radiosity results from the output files
# - get a file for a given timestamp

# Application layer doesn't have to know how the steps are divided. For instance simulation_service.py can simply 
# call repository.get_result_value(...).
# It implements the result_repository port.