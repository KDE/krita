# - Find PostgreSQL
# Find the PostgreSQL includes and client library
# This module defines
#  POSTGRESQL_INCLUDE_DIR, where to find POSTGRESQL.h
#  POSTGRESQL_LIBRARIES, the libraries needed to use POSTGRESQL.
#  POSTGRESQL_FOUND, If false, do not try to use PostgreSQL.

find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h
   /usr/include/pgsql
   /usr/local/include/pgsql
)

find_library(POSTGRESQL_LIBRARIES NAMES pq
   PATHS
   /usr/lib
   /usr/local/lib
)

if(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
   set(POSTGRESQL_FOUND TRUE)
   message(STATUS "Found PostgreSQL: ${POSTGRESQL_INCLUDE_DIR}, ${POSTGRESQL_LIBRARIES}")
else(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
   set(POSTGRESQL_FOUND FALSE)
   message(STATUS "PostgreSQL not found.")
endif(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)

mark_as_advanced(POSTGRESQL_INCLUDE_DIR POSTGRESQL_LIBRARIES)

