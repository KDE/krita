# - Find libpqxx
# Find the libpqxx includes and client library
# This module defines
#  PQXX_INCLUDE_DIR, where to find pqxx header file
#  PQXX_LIBRARIES, the libraries needed to use libpqxx.
#  PQXX_FOUND, If false, do not try to use libpqxx.

find_path(PQXX_INCLUDE_DIR pqxx
   /usr/include/pqxx
   /usr/local/include/pqxx
)

find_library(PQXX_LIBRARIES NAMES pqxx
   PATHS
   /usr/lib
   /usr/local/lib
)

if(PQXX_INCLUDE_DIR AND PQXX_LIBRARIES)
   set(PQXX_FOUND TRUE)
   message(STATUS "Found libpqxx: ${PQXX_INCLUDE_DIR}, ${PQXX_LIBRARIES}")
else(PQXX_INCLUDE_DIR AND PQXX_LIBRARIES)
   set(PQXX_FOUND FALSE)
   message(STATUS "libpqxx not found.")
endif(PQXX_INCLUDE_DIR AND PQXX_LIBRARIES)

mark_as_advanced(PQXX_INCLUDE_DIR PQXX_LIBRARIES)

