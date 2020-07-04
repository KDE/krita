# Find SIP
# ~~~~~~~~
#
# SIP website: http://www.riverbankcomputing.co.uk/sip/index.php
#
# Find the installed version of SIP. FindSIP should be called after Python
# has been found.
#
# This file defines the following variables:
#
# SIP_VERSION_STR - The version of SIP found as a human readable string.
#
# SIP_EXECUTABLE - Path and filename of the SIP command line executable.
#
# SIP_MODULE_EXECUTABLE - Path and filename of the sip-module executable.

# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



IF(SIP_VERSION_STR)
  # Already in cache, be silent
  SET(SIP_FOUND TRUE)
ELSE(SIP_VERSION_STR)

  find_program(SIP_EXECUTABLE NAMES sip5 sip)
  find_program(SIP_MODULE_EXECUTABLE sip-module)
  macro_bool_to_01(SIP_EXECUTABLE SIP_FOUND)

  IF(SIP_FOUND)
    execute_process(COMMAND ${SIP_EXECUTABLE} -V OUTPUT_VARIABLE SIP_VERSION_STR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT SIP_FIND_QUIETLY)
      MESSAGE(STATUS "Found SIP version: ${SIP_VERSION_STR}")
    ENDIF(NOT SIP_FIND_QUIETLY)
  ELSE(SIP_FOUND)
    IF(SIP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SIP")
    ENDIF(SIP_FIND_REQUIRED)
  ENDIF(SIP_FOUND)

ENDIF(SIP_VERSION_STR)
