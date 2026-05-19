# So far the solCommand.py has been working with one big mutable object with .param and .carac. dto.py should contain the definition of data transfer objects (DTO) to pass data between different parts of the code. This will help to decouple the code and make it more modular and maintainable. The DTO can be defined as a class with attributes corresponding to the parameters and characteristics needed for the solCommand.py to function properly.
# A) SoleneRunRequest: Running certain type of Solene calculation with specific parameters and characteristics.
# B) SolenePreparedInputs: Created upon data preparation, containing all the necessary information and file paths for running Solene.
# C) SoleneCommandSpec: A structured representation of the command to be executed, including the command line arguments, environment variables, and any other relevant information needed to run Solene.
# D) SoleneExecutionResult: A structured representation of the results obtained from running Solene, including output data, logs, and any relevant metadata.
# E) SoleneParsedOutput: A structured representation of the parsed output from Solene, containing the relevant information extracted from the raw output files.

# Why all this?
# 1) Obtaining a stable integrational contact point between different modules of the code, such as input preparation, process running, output parsing, and path resolution.
# 2) Facilitating the testing and debugging of individual components by providing clear data structures for input and output.
# 3) Logging clearence and traceability of data flow through the system, making it easier to identify where issues may arise.