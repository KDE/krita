# Copyright (c) 2010, Cyrille Berger, <cberger@cberger.net>
# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_custom_target(benchmark)

MACRO (CALLIGRA_ADD_BENCHMARK _test_NAME)

    set(_srcList ${ARGN})
    set(_targetName ${_test_NAME})
    if( ${ARGV1} STREQUAL "TESTNAME" )
        set(_targetName ${ARGV2})
        list(REMOVE_AT _srcList 0 1)
    endif( ${ARGV1} STREQUAL "TESTNAME" )
    
    set(_nogui)
    list(GET ${_srcList} 0 first_PARAM)
    if( ${first_PARAM} STREQUAL "NOGUI" )
        set(_nogui "NOGUI")
    endif( ${first_PARAM} STREQUAL "NOGUI" )

    kde4_add_executable( ${_test_NAME} TEST ${_srcList} )

    if(NOT KDE4_TEST_OUTPUT)
        set(KDE4_TEST_OUTPUT plaintext)
    endif(NOT KDE4_TEST_OUTPUT)
    set(KDE4_TEST_OUTPUT ${KDE4_TEST_OUTPUT} CACHE STRING "The output to generate when running the QTest unit tests")

    set(using_qtest "")
    foreach(_filename ${_srcList})
        if(NOT using_qtest)
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
                file(READ ${_filename} file_CONTENT)
                string(REGEX MATCH "QTEST_(KDE)?MAIN" using_qtest "${file_CONTENT}")
            endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
        endif(NOT using_qtest)
    endforeach(_filename)

    get_target_property( loc ${_test_NAME} LOCATION )
    if(WIN32)
      if(MSVC_IDE)
        STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}" loc "${loc}")
      endif()
      # .bat because of rpath handling
      set(_executable "${loc}.bat")
    else(WIN32)
      if (Q_WS_MAC AND NOT _nogui)
        set(_executable ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME}.app/Contents/MacOS/${_test_NAME})
      else (Q_WS_MAC AND NOT _nogui)
        # .shell because of rpath handling
        set(_executable "${loc}.shell")
      endif (Q_WS_MAC AND NOT _nogui)
    endif(WIN32)
    
    if (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : Using QTestLib, can produce XML report.")
        add_custom_target( ${_targetName} COMMAND ${_executable} -xml -o ${_targetName}.tml )
    else (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : NOT using QTestLib, can't produce XML report, please use QTestLib to write your unit tests.")
        add_custom_target( ${_targetName} COMMAND ${_executable} )
    endif (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")

    add_dependencies(benchmark ${_targetName} )
#    add_test( ${_targetName} ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME} -xml -o ${_test_NAME}.tml )

    if (NOT MSVC_IDE)   #not needed for the ide
        # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
        if (NOT KDE4_BUILD_TESTS)
           get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
           if(NOT _buildtestsAdded)
              add_custom_target(buildtests)
              set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
           endif(NOT _buildtestsAdded)
           add_dependencies(buildtests ${_test_NAME})
        endif (NOT KDE4_BUILD_TESTS)
    endif (NOT MSVC_IDE)

ENDMACRO (CALLIGRA_ADD_BENCHMARK)
