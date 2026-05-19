# Transforming Solene's outputs of solCommand.py and solFile.py into Python objects.
# 1) knows which file is to be opened for which result
# 2) knows which type of data is expected in which file
# 3) loads the content of the file
# 4) divides it into relevant sections
# 5) parses individual values
# 6) returns the results in a structured format (e.g., dictionaries, lists, custom classes)

# Answers the question "how do I get meaningful data of a certain type from a certain file?". For instance, 
# how do I get the albedo value from the .val file? Or how do I get the radiosity results from the output files?

# Finds a row with a certain keyword, then uses a suitable parser from value_parser.py to extract the value.