#
# SPDX-License-Identifier: BSD-3-Clause
#

include(CMakeParseArguments)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(KritaTestSuite)


# modified version of ECMAddTests.cmake in cmake-extra-modules
function(KRITA_ADD_UNIT_TEST)
  set(options GUI BROKEN)
  # TARGET_NAME_VAR and TEST_NAME_VAR are undocumented args used by
  # kis_add_tests
  set(oneValueArgs TEST_NAME NAME_PREFIX TARGET_NAME_VAR TEST_NAME_VAR PCH_FILE)
  set(multiValueArgs LINK_LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(_sources ${ARG_UNPARSED_ARGUMENTS})
  list(LENGTH _sources _sourceCount)
  if(ARG_TEST_NAME)
    set(_targetname ${ARG_TEST_NAME})
  elseif(${_sourceCount} EQUAL "1")
    #use the source file name without extension as the testname
    get_filename_component(_targetname ${_sources} NAME_WE)
  else()
    #more than one source file passed, but no test name given -> error
    message(FATAL_ERROR "kis_add_test() called with multiple source files but without setting \"TEST_NAME\"")
  endif()

  set(_testname ${ARG_NAME_PREFIX}${_targetname})

  if(ARG_BROKEN)
      # add test to the global list of disabled tests
      set(KRITA_BROKEN_TESTS ${KRITA_BROKEN_TESTS} ${_testname} CACHE INTERNAL "KRITA_BROKEN_TESTS")
  endif()
  # add test to global test list so we can operate on all targets at once
  set(KRITA_TESTS_TARGET ${KRITA_TESTS_TARGET} ${_targetname} CACHE INTERNAL "KRITA_TESTS_TARGET")

  set(gui_args)
  if(ARG_GUI)
      set(gui_args WIN32 MACOSX_BUNDLE)
  endif()
  add_executable(${_targetname} ${gui_args} ${_sources})  
  set_test_sdk_compile_definitions(${_targetname})

  if (KRITA_ENABLE_PCH AND ARG_PCH_FILE)
      set_property(TARGET ${_targetname} PROPERTY PCH_WARN_INVALID TRUE )
      target_precompile_headers(${_targetname} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch/${LOCAL_PCH_FILE}>")
  endif()

  if(NOT ARG_GUI)
    ecm_mark_nongui_executable(${_targetname})
  endif()

  # do not add it as test, so make test skips it unless asked for it
  if(NOT ARG_BROKEN OR KRITA_ENABLE_BROKEN_TESTS)
    add_test(NAME ${_testname} COMMAND ${_targetname})
  endif()

  target_link_libraries(${_targetname} ${ARG_LINK_LIBRARIES})
  ecm_mark_as_test(${_targetname})
  if (ARG_TARGET_NAME_VAR)
    set(${ARG_TARGET_NAME_VAR} "${_targetname}" PARENT_SCOPE)
  endif()
  if (ARG_TEST_NAME_VAR)
    set(${ARG_TEST_NAME_VAR} "${_testname}" PARENT_SCOPE)
  endif()
endfunction()

function(KRITA_ADD_UNIT_TESTS)
  set(options GUI BROKEN)
  set(oneValueArgs NAME_PREFIX TARGET_NAMES_VAR TEST_NAMES_VAR PCH_FILE)
  set(multiValueArgs LINK_LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(ARG_GUI)
    set(_exe_type GUI)
  else()
    set(_exe_type "")
  endif()

  set(BROKEN_TOKEN)
  if (ARG_BROKEN)
      set(BROKEN_TOKEN BROKEN)
  endif()

  set(test_names)
  set(target_names)

  set(pch_source_target)

  list(LENGTH ARG_UNPARSED_ARGUMENTS _testsCount)

  foreach(_test_source ${ARG_UNPARSED_ARGUMENTS})
    KRITA_ADD_UNIT_TEST(${_test_source}
      NAME_PREFIX ${ARG_NAME_PREFIX}
      LINK_LIBRARIES ${ARG_LINK_LIBRARIES}
      TARGET_NAME_VAR target_name
      TEST_NAME_VAR test_name
      ${_exe_type}
      ${BROKEN_TOKEN}
    )
    list(APPEND _test_names "${test_name}")
    list(APPEND _target_names "${target_name}")

    # if the number of tests is greater than 2 we
    # use the first test as a base for the PCH file
    # and then reuse this PCH for all other tests

    if (KRITA_ENABLE_PCH AND ${_testsCount} GREATER "2")
        set_property(TARGET ${_targetname} PROPERTY PCH_WARN_INVALID TRUE )

        if (NOT pch_source_target)
            set (pch_source_target ${target_name})

            if (NOT ARG_PCH_FILE)
                krita_select_pch_file(${target_name} ARG_PCH_FILE)
                #file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/out/${target_name}.txt" CONTENT ${ARG_PCH_FILE})
            endif()

            target_precompile_headers(${target_name} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/pch/${ARG_PCH_FILE}>")
        else()
            target_precompile_headers(${target_name} REUSE_FROM ${pch_source_target})
        endif()
    endif()

  endforeach()
  if (ARG_TARGET_NAMES_VAR)
    set(${ARG_TARGET_NAMES_VAR} "${_target_names}" PARENT_SCOPE)
  endif()
  if (ARG_TEST_NAMES_VAR)
    set(${ARG_TEST_NAMES_VAR} "${_test_names}" PARENT_SCOPE)
  endif()
endfunction()

macro(KRITA_ADD_BROKEN_UNIT_TEST)
    KRITA_ADD_UNIT_TEST(${ARGV} BROKEN)
endmacro()

macro(KRITA_ADD_BROKEN_UNIT_TESTS)
    KRITA_ADD_UNIT_TESTS(${ARGV} BROKEN)
endmacro()

macro(kis_add_tests)
    KRITA_ADD_UNIT_TESTS(${ARGV})
endmacro()

macro(kis_add_test)
    KRITA_ADD_UNIT_TEST(${ARGV})
endmacro()
