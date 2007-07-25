if (GLPK_INCLUDE_DIR)
  # in cache already
  set(GLPK_FOUND TRUE)
else (GLPK_INCLUDE_DIR)

find_path(GLPK_INCLUDE_DIR NAMES glpk.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
     ${GLPK_INCLUDE_PATH}
     /usr/include
     /usr/local/include
   )

if(GLPK_INCLUDE_DIR)
  set(GLPK_FOUND TRUE)
endif(GLPK_INCLUDE_DIR)

if(GLPK_FOUND)
   if(NOT GLPK_FIND_QUIETLY)
      message(STATUS "Found GLPK: ${GLPK_INCLUDE_DIR}")
   endif(NOT GLPK_FIND_QUIETLY)
else(GLPK_FOUND)
   if(GLPK_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find GLPK")
   endif(GLPK_FIND_REQUIRED)
endif(GLPK_FOUND)

mark_as_advanced(GLPK_INCLUDE_DIR)

endif(GLPK_INCLUDE_DIR)
