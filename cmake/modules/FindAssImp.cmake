# Try to find Asset Importer (AssImp) libraries
# Once done this will define
#
#  ASSIMP_FOUND       - System has AssImp
#  ASSIMP_INCLUDE_DIR - Include directory for AssImp
#  ASSIMP_LIBRARY     - The AssImp Library
#

find_path(ASSIMP_INCLUDE_DIR
    assimp.hpp
    PATH_SUFFIXES
    assimp
)

find_library(ASSIMP_LIBRARY
    NAMES assimp
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AssImp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)