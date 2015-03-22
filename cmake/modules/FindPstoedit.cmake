# - Try to find Pstoedit
# Once done this will define
#
#  PSTOEDIT_FOUND - system has pstoedit
#  PSTOEDIT_EXECUTABLE - path of the pstoedit executable
#  PSTOEDIT_VERSION - the version string, like "3.45"
#  PSTOEDIT_SVG_PLUGIN_FOUND - svg output plugin found

find_program(PSTOEDIT_EXECUTABLE NAMES pstoedit)

include(MacroEnsureVersion)

if(PSTOEDIT_EXECUTABLE)
    set(PSTOEDIT_FOUND TRUE)

    execute_process(COMMAND ${PSTOEDIT_EXECUTABLE} -help
        ERROR_VARIABLE _PSTOEDIT_VERSION OUTPUT_VARIABLE _PSTOEDIT_PLUGINS
    )
    string (REGEX MATCH "[0-9]\\.[0-9]+" PSTOEDIT_VERSION "${_PSTOEDIT_VERSION}")
    string (REGEX MATCH "plot-svg" PSTOEDIT_SVG_PLUGIN "${_PSTOEDIT_PLUGINS}")
    string (COMPARE EQUAL "plot-svg" "${PSTOEDIT_SVG_PLUGIN}" PSTOEDIT_SVG_PLUGIN_FOUND )
endif()

if(PSTOEDIT_FOUND AND PSTOEDIT_SVG_PLUGIN_FOUND )
  if(NOT Pstoedit_FIND_QUIETLY)
    message(STATUS "Found pstoedit version ${PSTOEDIT_VERSION}: ${PSTOEDIT_EXECUTABLE}")
    message(STATUS "Found pstoedit svg-plugin: ${PSTOEDIT_SVG_PLUGIN}")
  endif()
else()
  if(Pstoedit_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find pstoedit or the svg output plugin")
  endif()
endif()

