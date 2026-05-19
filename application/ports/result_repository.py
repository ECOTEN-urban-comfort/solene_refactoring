# Persistence and loading of the results of the simulation.
# In general the results are scattered amongst different folders, files, binary, intermediate
# results, in cache. The application service should not know about the details of where the results are stored.
# It should just be able to save the results.

# So this module should know:
# 1) where the results are stored
# 2) how to save the results
# 3) how to load the results (if needed)

# Not having this module would make the application service check if the file exists, compose file paths, open files.

# thus
# result_repository = application port of what the app wants
# file_repository = technical helper for the work with files
# result_locator = technical logic for where the results are stored
# output_reader = technical logic for how to read the results from the files
# value_parser = technical logic for how to parse the values from the files