# Central place for shared simulation constants.
#
# This module is intended to collect constant values that are used across the
# codebase and should not be repeated as inline literals.
#
# Typical examples include:
# - supported model names,
# - default filenames,
# - file format identifiers,
# - runtime folder names,
# - artifact type names,
# - other stable symbolic values needed by multiple modules.
#
# Keeping such values here helps reduce duplication, avoid drift between files,
# and make future renaming or extension safer.