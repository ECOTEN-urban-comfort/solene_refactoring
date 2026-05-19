# This layer deals with renewability and inkremental continuation of the Solene process.
# It's capable of:
# - tracking what has been computed
# - what still needs to be computed
# - what's invalid and needs to be recomputed
# - from which step to continue the process
# - if the process can be restored safely or if it needs to be restarted from the beginning

# Questions like:
# - A file exists, but 
#           - is outdated
#           - comes from a different configuration
#           - an itnermediate output exists, but the final output is missing
#           - a geometry has changed, the previous coupling results are invalid