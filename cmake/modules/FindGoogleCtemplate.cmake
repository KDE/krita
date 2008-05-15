# Locate: google/ctemplate
#
# Author: pepone.onrez@gmail.com
# License: GPL-v3
#
# This cmake script searchs for the google/ctemplate c++ package.
# Note that i only test it on gentoo/linux you must need add other paths
# for other Os. Also we must need same test for guest the coorect version is installed.
#
# start with 'not found'
# If you has any suggestions fell free to post in the user group
# http://groups.google.com/group/ydra
#

SET( GOOGLE_CTEMPLATE_FOUND 0 CACHE BOOL "Do we have Google CTemplate?" )

FIND_PATH(GOOGLE_CTEMPLATE_INCLUDE_DIR google/template.h )

FIND_LIBRARY(GOOGLE_CTEMPLATE_THREAD_LIB ctemplate )
FIND_LIBRARY(GOOGLE_CTEMPLATE_NOTHREAD_LIB ctemplate_nothreads  )

FIND_PROGRAM(GOOGLE_CTEMPLATE_COMPILER make_tpl_varnames_h )

#How guest if all this are found

IF ( GOOGLE_CTEMPLATE_INCLUDE_DIR)
    SET( GOOGLE_CTEMPLATE_FOUND 1 CACHE BOOL "Do we have Google CTemplate?" FORCE )
    MESSAGE(STATUS "google/ctemplate include files found at: ${GOOGLE_CTEMPLATE_INCLUDE_DIR}")
ENDIF( GOOGLE_CTEMPLATE_INCLUDE_DIR )

#If include are there continue and check if libs are there.
IF(GOOGLE_CTEMPLATE_FOUND)
    #Check if the system has not the ctemplate thread lib
    IF (NOT GOOGLE_CTEMPLATE_THREAD_LIB)
        SET( GOOGLE_CTEMPLATE_THREADS ' CHACHE BOOL)
        MESSAGE(STATUS "Your ctemplate seems that has not thread support")

        #Check if the project require the thread lib
        IF(GOOGLE_CTEMPLATE_USE_THREAD)
            SET( GOOGLE_CTEMPLATE_FOUND 0 CACHE BOOL "Do we have Google CTemplate?" FORCE )
            MESSAGE(STATUS "Your project requires Google CTemplate with thread support but your package seems is build without thread support")
        ENDIF(GOOGLE_CTEMPLATE_USE_THREAD)

        #Check if at least we have the no thread version
        IF(NOT GOOGLE_CTEMPLATE_NOTHREAD_LIB)
            SET( GOOGLE_CTEMPLATE_FOUND 0 CACHE BOOL "Do we have Google CTemplate?" FORCE )
            MESSAGE( STATUS "Your system doesn't have any Google CTemplate libs installed")
        ELSE(NOT GOOGLE_CTEMPLATE_THREAD_LIB)
            MESSAGE(STATUS "Google CTemplate nothreadlib found at: ${GOOGLE_CTEMPLATE_NOTHREAD_LIB}")
        ENDIF(NOT GOOGLE_CTEMPLATE_NOTHREAD_LIB)
    ELSE(NOT GOOGLE_CTEMPLATE_THREAD_LIB)
        MESSAGE(STATUS "Google CTemplate threadlib found at: ${GOOGLE_CTEMPLATE_THREAD_LIB}")
    ENDIF(NOT GOOGLE_CTEMPLATE_THREAD_LIB)
ENDIF(GOOGLE_CTEMPLATE_FOUND)

#If libs and include are in place Check that template compiler is also there.
#We need same way for test the compiler version
IF(GOOGLE_CTEMPLATE_FOUND AND GOOGLE_CTEMPLATE_USE_COMPILER)
    IF(NOT GOOGLE_CTEMPLATE_COMPILER)
        #The template compiler is not there
        MESSAGE( STATUS "Your system doesn't have the Google CTemplate compiler")
        SET( GOOGLE_CTEMPLATE_FOUND 0 CACHE BOOL "Do we have Google CTemplate?" FORCE )
    ELSE(NOT GOOGLE_CTEMPLATE_COMPILER)
        MESSAGE( STATUS "Google CTemplate compiler found at: ${GOOGLE_CTEMPLATE_COMPILER}")
    ENDIF(NOT GOOGLE_CTEMPLATE_COMPILER)
ENDIF(GOOGLE_CTEMPLATE_FOUND AND GOOGLE_CTEMPLATE_USE_COMPILER)

IF(GoogleCtemplate_FIND_REQUIRED)
  message(FATAL_ERROR "Required package Google CTemplate NOT found")
ELSE(GoogleCtemplate_FIND_REQUIRED)
  message(STATUS "Could not find OPTIONAL package Google CTemplate")
ENDIF(GoogleCtemplate_FIND_REQUIRED)
