# for documentation look at FindKDE4Internal.cmake

# this file contains the following macros (or functions):
# KDE4_ADD_UI_FILES
# KDE4_ADD_UI3_FILES
# KDE4_ADD_KCFG_FILES
# _KDE4_SET_CUSTOM_TARGET_PROPERTY
# _KDE4_GET_CUSTOM_TARGET_PROPERTY
# KDE4_ADD_PLUGIN
# KDE4_ADD_KDEINIT_EXECUTABLE
# KDE4_ADD_UNIT_TEST
# KDE4_ADD_EXECUTABLE
# KDE4_ADD_WIDGET_FILES
# KDE4_UPDATE_ICONCACHE
# KDE4_INSTALL_ICONS
# KDE4_REMOVE_OBSOLETE_CMAKE_FILES
# KDE4_CREATE_HANDBOOK
# KDE4_ADD_APP_ICON
# KDE4_CREATE_MANPAGE
# KDE4_CREATE_BASIC_CMAKE_VERSION_FILE (function)
# KDE4_INSTALL_AUTH_HELPER_FILES
# KDE4_AUTH_INSTALL_ACTIONS

# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


set(KDE4_MODULE_DIR  ${CMAKE_CURRENT_LIST_DIR} )

# This macro doesn't set up the RPATH related options for executables anymore,
# since now (wioth cmake 2.6) just the full RPATH is used always for everything.
# It does create wrapper shell scripts for the executables.
# It overrides the defaults set in FindKDE4Internal.cmake.
# For every executable a wrapper script is created, which sets the appropriate
# environment variable for the platform (LD_LIBRARY_PATH on most UNIX systems,
# DYLD_LIBRARY_PATH on OS X and PATH in Windows) so  that it points to the built
# but not yet installed versions of the libraries. So if RPATH is disabled, the executables
# can be run via these scripts from the build tree and will find the correct libraries.
# If RPATH is not disabled, these scripts are also used but only for consistency, because
# they don't really influence anything then, because the compiled-in RPATH overrides
# the LD_LIBRARY_PATH env. variable.
macro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE _target_NAME)
   if (UNIX)
      if (APPLE)
         set(_library_path_variable "DYLD_LIBRARY_PATH")
      elseif (CYGWIN)
         set(_library_path_variable "PATH")
      else (APPLE)
         set(_library_path_variable "LD_LIBRARY_PATH")
      endif (APPLE)

      if (APPLE)
         # DYLD_LIBRARY_PATH does not work like LD_LIBRARY_PATH
         # OSX already has the RPATH in libraries and executables, putting runtime directories in
         # DYLD_LIBRARY_PATH actually breaks things
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${KDE4_LIB_DIR}")
      else (APPLE)
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${LIB_INSTALL_DIR}:${KDE4_LIB_DIR}:${QT_LIBRARY_DIR}")
      endif (APPLE)
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the sh-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
         -D_ld_library_path="${_ld_library_path}" -D_executable=$<TARGET_FILE:${_target_NAME}>
         -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
         )

      macro_additional_clean_files(${_executable}.shell)

      # under UNIX, set the property WRAPPER_SCRIPT to the name of the generated shell script
      # so it can be queried and used later on easily
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable}.shell)

   else (UNIX)
      # under windows, set the property WRAPPER_SCRIPT just to the name of the executable
      # maybe later this will change to a generated batch file (for setting the PATH so that the Qt libs are found)
      get_target_property(_executable ${_target_NAME} LOCATION )
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable})

      set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}\;${LIB_INSTALL_DIR}\;${KDE4_LIB_DIR}\;${QT_LIBRARY_DIR}")
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the batch-file-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename="${_executable}.bat"
         -D_ld_library_path="${_ld_library_path}" -D_executable="${_executable}"
         -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
         )

   endif (UNIX)
endmacro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE)


macro (KDE4_ADD_PLUGIN _target_NAME )
#if the first argument is "WITH_PREFIX" then keep the standard "lib" prefix,
#otherwise set the prefix empty

   set(_args ${ARGN})
   # default to module
   set(_add_lib_param "MODULE")
   set(_with_pre FALSE)

   foreach(arg ${_args})
      if (arg STREQUAL "WITH_PREFIX")
         set(_with_pre TRUE)
      endif (arg STREQUAL "WITH_PREFIX")
      if (arg STREQUAL "STATIC")
         set(_add_lib_param STATIC)
      endif (arg STREQUAL "STATIC")
      if (arg STREQUAL "SHARED")
         set(_add_lib_param SHARED)
      endif (arg STREQUAL "SHARED")
      if (arg STREQUAL "MODULE")
         set(_add_lib_param MODULE)
      endif (arg STREQUAL "MODULE")
   endforeach(arg)

   if(_with_pre)
      list(REMOVE_ITEM _args "WITH_PREFIX")
   endif(_with_pre)
   if(_add_lib_param STREQUAL "STATIC")
      list(REMOVE_ITEM _args "STATIC")
   endif(_add_lib_param STREQUAL "STATIC")
   if (_add_lib_param STREQUAL "SHARED")
      list(REMOVE_ITEM _args "SHARED")
   endif (_add_lib_param STREQUAL "SHARED")
   if (_add_lib_param STREQUAL "MODULE")
       list(REMOVE_ITEM _args "MODULE")
   endif (_add_lib_param STREQUAL "MODULE")

   set(_SRCS ${_args})

   if("${_add_lib_param}" STREQUAL "STATIC")
      add_definitions(-DQT_STATICPLUGIN)
   endif("${_add_lib_param}" STREQUAL "STATIC")

   add_library(${_target_NAME} ${_add_lib_param}  ${_SRCS})

   if (NOT _with_pre)
      set_target_properties(${_target_NAME} PROPERTIES PREFIX "")
   endif (NOT _with_pre)

   # for shared libraries/plugins a -DMAKE_target_LIB is required
   string(TOUPPER ${_target_NAME} _symbol)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
   set(_symbol "MAKE_${_symbol}_LIB")
   set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})

endmacro (KDE4_ADD_PLUGIN _target_NAME _with_PREFIX)


# this macro is intended to check whether a list of source
# files has the "NOGUI" or "RUN_UNINSTALLED" keywords at the beginning
# in _output_LIST the list of source files is returned with the "NOGUI"
# and "RUN_UNINSTALLED" keywords removed
# if "NOGUI" is in the list of files, the _nogui argument is set to
# "NOGUI" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
# "RUN_UNINSTALLED" in the list of files is ignored, it is not necessary anymore
# since KDE 4.2 (with cmake 2.6.2), since then all executables are always built
# with RPATH pointing into the build dir.
# if "TEST" is in the list of files, the _test argument is set to
# "TEST" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
macro(KDE4_CHECK_EXECUTABLE_PARAMS _output_LIST _nogui _test)
   set(${_nogui})
   set(${_test})
   set(${_output_LIST} ${ARGN})
   list(LENGTH ${_output_LIST} count)

   list(GET ${_output_LIST} 0 first_PARAM)

   set(second_PARAM "NOTFOUND")
   if (${count} GREATER 1)
      list(GET ${_output_LIST} 1 second_PARAM)
   endif (${count} GREATER 1)

   set(remove "NOTFOUND")

   if (${first_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "NOGUI")

   if (${first_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${first_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "TEST")

   if (${second_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "NOGUI")

   if (${second_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${second_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "TEST")


   if (NOT "${remove}" STREQUAL "NOTFOUND")
      list(REMOVE_AT ${_output_LIST} ${remove})
   endif (NOT "${remove}" STREQUAL "NOTFOUND")

endmacro(KDE4_CHECK_EXECUTABLE_PARAMS)


macro (KDE4_ADD_KDEINIT_EXECUTABLE _target_NAME )

   kde4_check_executable_params(_SRCS _nogui _test ${ARGN})

   configure_file(${KDE4_MODULE_DIR}/kde4init_dummy.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp)
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp PROPERTIES SKIP_AUTOMOC TRUE)
   # under Windows, build a normal executable and additionally a dummy kdeinit4_foo.lib, whose only purpose on windows is to
   # keep the linking logic from the CMakeLists.txt on UNIX working (under UNIX all necessary libs are linked against the kdeinit
   # library instead against the executable, under windows we want to have everything in the executable, but for compatibility we have to
   # keep the library there-
   if(WIN32)
      if (MINGW)
         list(FIND _SRCS ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_res.o _res_position)
      else(MINGW)
         list(FIND _SRCS ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}.rc _res_position)
      endif(MINGW)
      if(NOT _res_position EQUAL -1)
         list(GET _SRCS ${_res_position} _resourcefile)
         list(REMOVE_AT _SRCS ${_res_position})
      endif(NOT _res_position EQUAL -1)

      set(_KDEINIT4_TARGET_NAME_ ${_target_NAME})
      string(REGEX REPLACE "[-]" "_" _KDEINIT4_TARGET_NAME_ "${_KDEINIT4_TARGET_NAME_}")
      configure_file(${KDE4_MODULE_DIR}/kde4init_win32lib_dummy.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_win32lib_dummy.cpp)
      add_library(kdeinit_${_target_NAME} STATIC ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_win32lib_dummy.cpp)

      kde4_add_executable(${_target_NAME} "${_nogui}" ${_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${_resourcefile})

      set_target_properties(kdeinit_${_target_NAME} PROPERTIES OUTPUT_NAME kdeinit4_${_target_NAME})

      target_link_libraries(${_target_NAME} kdeinit_${_target_NAME})
   else(WIN32)

      add_library(kdeinit_${_target_NAME} SHARED ${_SRCS})

      set_target_properties(kdeinit_${_target_NAME} PROPERTIES OUTPUT_NAME kdeinit4_${_target_NAME})

      if (Q_WS_MAC)
	      list(FIND _SRCS *.icns _icon_position)
	      if(NOT _res_position EQUAL -1)
		      list(GET _SRCS ${_icon_position} _resourcefile)
	      endif(NOT _res_position EQUAL -1)
      endif (Q_WS_MAC)
      kde4_add_executable(${_target_NAME} "${_nogui}" ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${_resourcefile})
      target_link_libraries(${_target_NAME} kdeinit_${_target_NAME})
   endif(WIN32)

endmacro (KDE4_ADD_KDEINIT_EXECUTABLE)

# Add a unit test, which is executed when running make test .
# The targets are always created, but only built for the "all"
# target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
# are created but not built by default. You can build them by manually building the target.
# The name of the target can be specified using TESTNAME <testname>, if it is not given
# the macro will default to the <name>
macro (KDE4_ADD_UNIT_TEST _test_NAME)
    set(_srcList ${ARGN})
    set(_targetName ${_test_NAME})
    if( ${ARGV1} STREQUAL "TESTNAME" )
        set(_targetName ${ARGV2})
        list(REMOVE_AT _srcList 0 1)
    endif( ${ARGV1} STREQUAL "TESTNAME" )

    set(_nogui)
    list(GET _srcList 0 first_PARAM)
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
        add_test( ${_targetName} ${_executable} -xml -o ${_targetName}.tml)
    else (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : NOT using QTestLib, can't produce XML report, please use QTestLib to write your unit tests.")
        add_test( ${_targetName} ${_executable} )
    endif (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")

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

endmacro (KDE4_ADD_UNIT_TEST)


# add a  manifest file to executables.
#
# There is a henn-egg problem when a target runtime part is renamed using
# the OUTPUT_NAME option of cmake's set_target_properties command.
#
# At now the Makefiles rules creating for manifest adding are performed
# *after* the cmake's add_executable command but *before* an optional
# set_target_properties command.
# This means that in KDE4_ADD_MANIFEST the LOCATION property contains
#  the unchanged runtime part name of the target. :-(
#
# The recently used workaround is to specify a variable build off the target name followed
# by _OUTPUT_NAME before calling kde4_add_executable as shown in the following example:
#
# set(xyz_OUTPUT_NAME test)
# kde4_add_executable( xyz <source>)
# set_target_properties( xyz PROPERTIES OUTPUT_NAME ${xyz_OUTPUT_NAME} )
#
# The full solution would be to introduce a kde4_target_link_libraries macro and to
# call KDE4_ADD_MANIFEST inside instead of calling in kde4_add_executable.
# This would require patching of *all* places in the KDE sources where target_link_libraries
# is used and to change the related docs.
#
# Because yet I found only 2 locations where this problem occurs (kjs, k3b), the workaround
# seems to be a pragmatically solution.
#
# This macro is an internal macro only used by kde4_add_executable
#
macro (_KDE4_ADD_MANIFEST _target_NAME)
    set(x ${_target_NAME}_OUTPUT_NAME)
    if (${x})
        get_target_property(_var ${_target_NAME} LOCATION )
        string(REPLACE "${_target_NAME}" "${${x}}" _executable ${_var})
    else(${x})
        get_target_property(_executable ${_target_NAME} LOCATION )
    endif(${x})

    if (_kdeBootStrapping)
        set(_cmake_module_path ${CMAKE_SOURCE_DIR}/cmake/modules)
    else (_kdeBootStrapping)
        set(_cmake_module_path ${KDE4_INSTALL_DIR}/share/apps/cmake/modules)
    endif (_kdeBootStrapping)

    set(_manifest ${KDE4_MODULE_DIR}/Win32.Manifest.in)
    #message(STATUS ${_executable} ${_manifest})
    add_custom_command(
        TARGET ${_target_NAME}
        POST_BUILD
        COMMAND ${KDE4_MT_EXECUTABLE}
        ARGS
           -manifest ${_manifest}
           -updateresource:${_executable}
        COMMENT "adding vista trustInfo manifest to ${_target_NAME}"
   )
endmacro(_KDE4_ADD_MANIFEST)


macro (KDE4_ADD_EXECUTABLE _target_NAME)

   kde4_check_executable_params( _SRCS _nogui _test ${ARGN})

   set(_add_executable_param)

   # determine additional parameters for add_executable()
   # for GUI apps, create a bundle on OSX
   if (Q_WS_MAC)
      set(_add_executable_param MACOSX_BUNDLE)
   endif (Q_WS_MAC)

   # for GUI apps, this disables the additional console under Windows
   if (WIN32)
      set(_add_executable_param WIN32)
   endif (WIN32)

   if (_nogui)
      set(_add_executable_param)
   endif (_nogui)

   if (_test AND NOT KDE4_BUILD_TESTS)
      set(_add_executable_param ${_add_executable_param} EXCLUDE_FROM_ALL)
   endif (_test AND NOT KDE4_BUILD_TESTS)

   add_executable(${_target_NAME} ${_add_executable_param} ${_SRCS})

   IF (KDE4_ENABLE_UAC_MANIFEST)
       _kde4_add_manifest(${_target_NAME})
   ENDIF(KDE4_ENABLE_UAC_MANIFEST)

   if (_test)
      set_target_properties(${_target_NAME} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
   endif (_test)

   kde4_handle_rpath_for_executable(${_target_NAME})

endmacro (KDE4_ADD_EXECUTABLE)

include(GenerateExportHeader)

macro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)

   set(_first_SRC ${_lib_TYPE})
   set(_add_lib_param)

   if (${_lib_TYPE} STREQUAL "STATIC")
      set(_first_SRC)
      set(_add_lib_param STATIC)
   endif (${_lib_TYPE} STREQUAL "STATIC")
   if (${_lib_TYPE} STREQUAL "SHARED")
      set(_first_SRC)
      set(_add_lib_param SHARED)
   endif (${_lib_TYPE} STREQUAL "SHARED")
   if (${_lib_TYPE} STREQUAL "MODULE")
      set(_first_SRC)
      set(_add_lib_param MODULE)
   endif (${_lib_TYPE} STREQUAL "MODULE")

   set(_SRCS ${_first_SRC} ${ARGN})

   add_library(${_target_NAME} ${_add_lib_param} ${_SRCS})

   # for shared libraries a -DMAKE_target_LIB is required
   string(TOUPPER ${_target_NAME} _symbol)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
   set(_symbol "MAKE_${_symbol}_LIB")
   set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})
endmacro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)

macro(kf5_add_library _target_NAME _lib_TYPE)
  kde4_add_library(${_target_NAME} ${_lib_TYPE} ${ARGN})
  generate_export_header(${_target_NAME})
endmacro()

# TODO: Move this to kde4support
#if (_kdeBootStrapping)
#  include("${CMAKE_SOURCE_DIR}/tier2/kauth/cmake/KAuthMacros.cmake")
#  include("${CMAKE_SOURCE_DIR}/kdecore/KDECoreMacros.cmake")
   include("${CMAKE_SOURCE_DIR}/cmake/kde_macro/KDEUIMacros.cmake")
#  include("${CMAKE_SOURCE_DIR}/kdoctools/KDocToolsMacros.cmake")
#else()
#include("${KDE4_LIB_DIR}/cmake/KAuth/KAuthMacros.cmake")
#  include("${CMAKE_CURRENT_LIST_DIR}/KDECoreMacros.cmake")
#  include("${CMAKE_CURRENT_LIST_DIR}/KDEUIMacros.cmake")
#  include("${CMAKE_CURRENT_LIST_DIR}/KDocToolsMacros.cmake")
#endif()
