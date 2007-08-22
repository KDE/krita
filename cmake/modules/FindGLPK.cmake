if (GLPK_INCLUDE_DIR)
  # in cache already
  set(GLPK_FOUND TRUE)
else (GLPK_INCLUDE_DIR)

find_path(GLPK_INCLUDE_DIR NAMES glpk.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
     ${GLPK_INCLUDE_PATH}
   )

find_library(GLPK_LIBRARY glpk)

if(GLPK_INCLUDE_DIR AND GLPK_LIBRARY)
  set(GLPK_FOUND TRUE)
endif(GLPK_INCLUDE_DIR AND GLPK_LIBRARY)

if(GLPK_FOUND)
   # Try to find the version using glp_version(), which was introduced in 4.16
   set(FIND_GLPK_VERSION_SOURCE
      "#include <stdio.h>\n#include <glpk.h>\n int main()\n {\n    printf(\"FOUND_GLPK_VERSION %s\\n\", glp_version());\n    return 0;\n}\n")
   set(FIND_GLPK_VERSION_SOURCE_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/FindGLPK.cxx)
   file(WRITE "${FIND_GLPK_VERSION_SOURCE_FILE}" "${FIND_GLPK_VERSION_SOURCE}")

   try_run(GLPK_RUN_RESULT GLPK_COMPILE_RESULT
      ${CMAKE_BINARY_DIR}
      ${FIND_GLPK_VERSION_SOURCE_FILE}
      CMAKE_FLAGS -DLINK_LIBRARIES:STRING=${GLPK_LIBRARY}
                  -DINCLUDE_DIRECTORIES:STRING=${GLPK_INCLUDE_DIR}
      OUTPUT_VARIABLE GLPK_VERSION_OUTPUT)

   if(GLPK_COMPILE_RESULT AND NOT GLPK_RUN_RESULT STREQUAL FAILED_TO_RUN)

      string(REGEX REPLACE ".*FOUND_GLPK_VERSION ([0-9]+\\.[0-9]+).*" "\\1" GLPK_VERSION_STRING "${GLPK_VERSION_OUTPUT}")
      string(REGEX REPLACE "^([0-9]+)\\.[0-9]+.*" "\\1" MAJOR_VERSION "${GLPK_VERSION_STRING}")
      string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" MINOR_VERSION "${GLPK_VERSION_STRING}")

      if(NOT GLPK_FIND_QUIETLY)
         message(STATUS "Found GLPK version ${MAJOR_VERSION}.${MINOR_VERSION}: ${GLPK_INCLUDE_DIR} ${GLPK_LIBRARY}")
      endif(NOT GLPK_FIND_QUIETLY)
   else(GLPK_COMPILE_RESULT AND NOT GLPK_RUN_RESULT STREQUAL FAILED_TO_RUN)
      if(NOT GLPK_FIND_QUIETLY)
         message(STATUS "Found GLPK but failed to find version (likely earlier than 4.16): ${GLPK_INCLUDE_DIR} ${GLPK_LIBRARY}")
         file(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
            "Find GLPK version failed with the following output:\n${GLPK_VERSION_OUTPUT}\nFindGLPK.cxx:\n${FIND_GLPK_VERSION_SOURCE}\n\n")
      endif(NOT GLPK_FIND_QUIETLY)
      set(MAJOR_VERSION 0)
      set(MINOR_VERSION 0)
   endif(GLPK_COMPILE_RESULT AND NOT GLPK_RUN_RESULT STREQUAL FAILED_TO_RUN)

   math(EXPR VERSION "${MAJOR_VERSION}*100 + ${MINOR_VERSION}")

   set(GLPK_MAJOR_VERSION ${MAJOR_VERSION} CACHE STRING "glpk major version")
   set(GLPK_MINOR_VERSION ${MINOR_VERSION} CACHE STRING "glpk minor version")
   set(GLPK_VERSION ${VERSION} CACHE STRING "glpk version")

else(GLPK_FOUND)
   if(GLPK_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find GLPK")
   endif(GLPK_FIND_REQUIRED)
endif(GLPK_FOUND)

mark_as_advanced(GLPK_INCLUDE_DIR)
mark_as_advanced(GLPK_LIBRARY)
mark_as_advanced(GLPK_MAJOR_VERSION)
mark_as_advanced(GLPK_MINOR_VERSION)
mark_as_advanced(GLPK_VERSION)

endif(GLPK_INCLUDE_DIR)

