# - Find libpqxx
# Find the libpqxx includes and client library
# This module defines
#  PQXX_INCLUDE_DIR, where to find pqxx header file
#  PQXX_LIBRARIES, the libraries needed to use libpqxx.
#  PQXX_FOUND, If false, do not try to use libpqxx.

FIND_PATH(PQXX_INCLUDE_DIR pqxx
PATHS
   /usr/include/pqxx
   /usr/local/include/pqxx
PATH_SUFFIXES
   pqxx
)

find_library(PQXX_LIBRARIES NAMES pqxx)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Pqxx DEFAULT_MSG PQXX_INCLUDE_DIR PQXX_LIBRARIES )

mark_as_advanced(PQXX_INCLUDE_DIR PQXX_LIBRARIES)

