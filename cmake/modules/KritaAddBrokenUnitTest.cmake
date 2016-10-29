include(CMakeParseArguments)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)


# modified version of ECMAddTests.cmake in cmake-extra-modules
function(KRITA_ADD_BROKEN_UNIT_TEST)
  set(options GUI)
  # TARGET_NAME_VAR and TEST_NAME_VAR are undocumented args used by
  # ecm_add_tests
  set(oneValueArgs TEST_NAME NAME_PREFIX TARGET_NAME_VAR TEST_NAME_VAR)
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
    message(FATAL_ERROR "ecm_add_test() called with multiple source files but without setting \"TEST_NAME\"")
  endif()

  set(_testname ${ARG_NAME_PREFIX}${_targetname})

  # add test to the global list of disabled tests
  set(KRITA_BROKEN_TESTS ${KRITA_BROKEN_TESTS} ${_testname} CACHE INTERNAL "KRITA_BROKEN_TESTS")

  set(gui_args)
  if(ARG_GUI)
      set(gui_args WIN32 MACOSX_BUNDLE)
  endif()
  add_executable(${_targetname} ${gui_args} ${_sources})
  if(NOT ARG_GUI)
    ecm_mark_nongui_executable(${_targetname})
  endif()

  # do not add it as test, so make test skips it unless asked for it
  if(KRITA_ENABLE_BROKEN_TESTS)
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
