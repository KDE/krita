if (GMM_INCLUDE_DIR)

  # in cache already
  set(GMM_FOUND TRUE)

else (GMM_INCLUDE_DIR)

find_path(GMM_INCLUDE_DIR NAMES gmm/gmm.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
     ${GMM_INCLUDE_PATH}
     /usr/include
     /usr/local/include
   )

if(GMM_INCLUDE_DIR)
  set(GMM_FOUND TRUE)
endif(GMM_INCLUDE_DIR)

if(GMM_FOUND)
   if(NOT GMM_FIND_QUIETLY)
      message(STATUS "Found GMM: ${GMM_INCLUDE_DIR}")
   endif(NOT GMM_FIND_QUIETLY)
else(GMM_FOUND)
   if(GMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find GMM")
   endif(GMM_FIND_REQUIRED)
endif(GMM_FOUND)

mark_as_advanced(GMM_INCLUDE_DIR)

endif(GMM_INCLUDE_DIR)
