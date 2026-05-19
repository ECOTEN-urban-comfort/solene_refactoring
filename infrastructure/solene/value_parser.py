# read_val() in solFile.py reads the .val file, from which a concrete value may be extracted
# - this module should provide a clear interface for retrieving specific values based on the file type and expected data format. It should also handle any necessary parsing and data transformation to ensure that the results are returned in a usable format for further processing or analysis.
# - "12,45" -> 12.45
# - "Average temperature: 21.8 C" -> 21.8
# - data type conversion from text to Python types (e.g., float, int, list, dict)