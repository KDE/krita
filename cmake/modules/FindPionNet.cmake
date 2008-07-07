#
# FindPionNet
# Looks for pion-network-library
#
# Usage:
# FIND_PACKAGE(PionNet)
#

# Try auto discovery
IF (NOT PionNet_FOUND)
    MESSAGE(STATUS "Looking for PionNet library")
    FIND_LIBRARY(PionNet_LIBRARY NAMES pion-net)
    FIND_PATH(PionNet_INCLUDE_DIR NAMES PionConfig.hpp
        PATHS /usr/include/pion /usr/local/include/pion)
ENDIF (NOT PionNet_FOUND)

IF (PionNet_INCLUDE_DIR AND PionNet_LIBRARY)
    SET(PionNet_FOUND TRUE)
    IF (NOT PionNet_FIND_QUIETLY)
        MESSAGE(STATUS "Found Pion Network Library: ${PionNet_LIBRARY}")
        MESSAGE(STATUS "Found Pion Network Library headers: ${PionNet_INCLUDE_DIR}")
    ENDIF (NOT PionNet_FIND_QUIETLY)
ELSE (PionNet_INCLUDE_DIR AND PionNet_LIBRARY)
    SET(PionNet_FOUND FALSE)
    MESSAGE(STATUS "Pion Network Library was NOT found")
ENDIF (PionNet_INCLUDE_DIR AND PionNet_LIBRARY)