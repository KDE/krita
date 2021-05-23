# Macros for SIP
# ~~~~~~~~~~~~~~
# SPDX-FileCopyrightText: 2007 Simon Edwards <simon@simonzone.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# SIP website: http://www.riverbankcomputing.co.uk/sip/index.php
#
# This file defines the following macros:
#
# ADD_SIP_PYTHON_MODULE (MODULE_NAME MODULE_SIP [library1, libaray2, ...])
#     Specifies a SIP file to be built into a Python module and installed.
#     MODULE_NAME is the name of Python module including any path name. (e.g.
#     os.sys, Foo.bar etc). MODULE_SIP the path and filename of the .sip file
#     to process and compile. libraryN are libraries that the Python module,
#     which is typically a shared library, should be linked to. The built
#     module will also be install into Python's site-packages directory.
#
# The behaviour of the ADD_SIP_PYTHON_MODULE macro can be controlled by a
# number of variables:
#
# SIP_INCLUDES - List of directories which SIP will scan through when looking
#     for included .sip files. (Corresponds to the -I option for SIP.)
#
# SIP_TAGS - List of tags to define when running SIP. (Corresponds to the -t
#     option for SIP.)
#
# SIP_CONCAT_PARTS - An integer which defines the number of parts the C++ code
#     of each module should be split into. Defaults to 8. (Corresponds to the
#     -j option for SIP.)
#
# SIP_DISABLE_FEATURES - List of feature names which should be disabled
#     running SIP. (Corresponds to the -x option for SIP.)
#
# SIP_EXTRA_OPTIONS - Extra command line options which should be passed on to
#     SIP.

SET(SIP_INCLUDES)
SET(SIP_TAGS)
SET(SIP_CONCAT_PARTS 8)
SET(SIP_DISABLE_FEATURES)
SET(SIP_EXTRA_OPTIONS)

if (${SIP_VERSION_STR} VERSION_LESS 5)

MACRO(ADD_SIP_PYTHON_MODULE MODULE_NAME MODULE_SIP)

    SET(EXTRA_LINK_LIBRARIES ${ARGN})

    STRING(REPLACE "." "/" _x ${MODULE_NAME})
    GET_FILENAME_COMPONENT(_parent_module_path ${_x}  PATH)
    GET_FILENAME_COMPONENT(_child_module_name ${_x} NAME)

    GET_FILENAME_COMPONENT(_module_path ${MODULE_SIP} PATH)

    if(_module_path STREQUAL "")
        set(CMAKE_CURRENT_SIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    else(_module_path STREQUAL "")
        set(CMAKE_CURRENT_SIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${_module_path}")
    endif(_module_path STREQUAL "")

    GET_FILENAME_COMPONENT(_abs_module_sip ${MODULE_SIP} ABSOLUTE)

    # We give this target a long logical target name.
    # (This is to avoid having the library name clash with any already
    # install library names. If that happens then cmake dependency
    # tracking get confused.)
    STRING(REPLACE "." "_" _logical_name ${MODULE_NAME})
    SET(_logical_name "python_module_${_logical_name}")

    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SIP_OUTPUT_DIR})    # Output goes in this dir.

    SET(_sip_includes)
    FOREACH (_inc ${SIP_INCLUDES})
        GET_FILENAME_COMPONENT(_abs_inc ${_inc} ABSOLUTE)
        LIST(APPEND _sip_includes -I ${_abs_inc})
    ENDFOREACH (_inc )

    SET(_sip_tags)
    FOREACH (_tag ${SIP_TAGS})
        LIST(APPEND _sip_tags -t ${_tag})
    ENDFOREACH (_tag)

    SET(_sip_x)
    FOREACH (_x ${SIP_DISABLE_FEATURES})
        LIST(APPEND _sip_x -x ${_x})
    ENDFOREACH (_x ${SIP_DISABLE_FEATURES})

    SET(_message "-DMESSAGE=Generating CPP code for module ${MODULE_NAME}")
    SET(_sip_output_files)
    FOREACH(CONCAT_NUM RANGE 0 ${SIP_CONCAT_PARTS} )
        IF( ${CONCAT_NUM} LESS ${SIP_CONCAT_PARTS} )
            SET(_sip_output_files ${_sip_output_files} ${CMAKE_CURRENT_SIP_OUTPUT_DIR}/sip${_child_module_name}part${CONCAT_NUM}.cpp )
        ENDIF( ${CONCAT_NUM} LESS ${SIP_CONCAT_PARTS} )
    ENDFOREACH(CONCAT_NUM RANGE 0 ${SIP_CONCAT_PARTS} )

    ADD_CUSTOM_COMMAND(
        OUTPUT ${_sip_output_files}
        COMMAND ${CMAKE_COMMAND} -E echo ${message}
        COMMAND ${CMAKE_COMMAND} -E touch ${_sip_output_files}
        COMMAND ${SIP_EXECUTABLE} ${_sip_tags} ${_sip_x} ${SIP_EXTRA_OPTIONS} -j ${SIP_CONCAT_PARTS} -c ${CMAKE_CURRENT_SIP_OUTPUT_DIR} ${_sip_includes} ${_abs_module_sip}
        DEPENDS ${_abs_module_sip} ${SIP_EXTRA_FILES_DEPEND}
    )
    # not sure if type MODULE could be uses anywhere, limit to cygwin for now
    IF (WIN32 OR CYGWIN OR APPLE)
        ADD_LIBRARY(${_logical_name} MODULE ${_sip_output_files} )
    ELSE (WIN32 OR CYGWIN OR APPLE)
        ADD_LIBRARY(${_logical_name} SHARED ${_sip_output_files} )
    ENDIF (WIN32 OR CYGWIN OR APPLE)
    TARGET_LINK_LIBRARIES(${_logical_name} ${PYTHON_LIBRARY})
    TARGET_LINK_LIBRARIES(${_logical_name} ${EXTRA_LINK_LIBRARIES})
    SET_TARGET_PROPERTIES(${_logical_name} PROPERTIES PREFIX "" OUTPUT_NAME ${_child_module_name})

    IF (MINGW)
        TARGET_COMPILE_DEFINITIONS(${_logical_name} PRIVATE _hypot=hypot)
    ENDIF (MINGW)

    IF (WIN32)
        SET_TARGET_PROPERTIES(${_logical_name} PROPERTIES SUFFIX ".pyd")
    ENDIF (WIN32)

    INSTALL(TARGETS ${_logical_name} DESTINATION "${PYTHON_SITE_PACKAGES_INSTALL_DIR}/${_parent_module_path}")

ENDMACRO(ADD_SIP_PYTHON_MODULE)

else()
    find_file(sip_generate "sip-generate.py" PATHS ${CMAKE_MODULE_PATH})
    find_file(pyproject_toml "pyproject.toml.in" PATHS ${CMAKE_MODULE_PATH})

    macro(add_sip_python_module_v5 MODULE_NAME MODULE_SIP)        
        get_filename_component(module_name_toml ${MODULE_SIP} NAME_WE)
        set(module_srcs "${SIP_EXTRA_FILES_DEPEND}")

        set(EXTRA_LINK_LIBRARIES ${ARGN})

        if (SIP_MODULE)
            set(sip_name ${SIP_MODULE})
        else()
            set(sip_name "sip")
        endif()

        set(module_tags)
        foreach(_tag ${SIP_TAGS})
            string(APPEND module_tags "\"${_tag}\",")
        endforeach()
        set(module_tags "[${module_tags}]")

        set(sip_include_dirs)
        foreach(_inc ${SIP_INCLUDES})
            get_filename_component(_abs_inc ${_inc} ABSOLUTE)
            string(APPEND sip_include_dirs "\"${_abs_inc}\",")
        endforeach()
        set(sip_include_dirs "[${sip_include_dirs}]")

        string(REPLACE "." "/" _x ${MODULE_NAME})
        get_filename_component(_parent_module_path ${_x} PATH)
        get_filename_component(_child_module_name ${_x} NAME)

        get_filename_component(_module_path ${MODULE_SIP} PATH)

        set(CMAKE_CURRENT_SIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/_tmp")

        get_filename_component(_abs_module_sip ${MODULE_SIP} ABSOLUTE)

        # We give this target a long logical target name.
        # (This is to avoid having the library name clash with any already
        # install library names. If that happens then cmake dependency
        # tracking get confused.)
        string(REPLACE "." "_" _logical_name ${MODULE_NAME})
        set(_logical_name "python_module_${_logical_name}")

        set(_sip_output_files)
        foreach(CONCAT_NUM RANGE 0 ${SIP_CONCAT_PARTS})
            if(${CONCAT_NUM} LESS ${SIP_CONCAT_PARTS})
                set(_sip_output_files ${_sip_output_files} ${CMAKE_CURRENT_SIP_OUTPUT_DIR}/${_child_module_name}/sip${_child_module_name}part${CONCAT_NUM}.cpp )
            endif( ${CONCAT_NUM} LESS ${SIP_CONCAT_PARTS})
        endforeach(CONCAT_NUM RANGE 0 ${SIP_CONCAT_PARTS})

        configure_file(
            ${pyproject_toml}
            ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml
        )
        add_custom_command(
            COMMAND
                ${CMAKE_COMMAND} -E echo "Generating SIP 5+ bindings for ${MODULE_NAME}..."
            COMMAND
                ${PYTHON_EXECUTABLE}
                ${sip_generate}
                --build-dir ${CMAKE_CURRENT_SIP_OUTPUT_DIR}
                --target-dir ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/${_parent_module_path}
                --concatenate ${SIP_CONCAT_PARTS}
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
                ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml
            OUTPUT
                ${_sip_output_files}
        )

        # not sure if type MODULE could be usec anywhere, limit to cygwin for now
        if (WIN32 OR CYGWIN OR APPLE)
            add_library(${_logical_name} MODULE ${_sip_output_files})
        else ()
            add_library(${_logical_name} SHARED ${_sip_output_files})
        endif ()
        target_include_directories(${_logical_name} PRIVATE ${CMAKE_CURRENT_SIP_OUTPUT_DIR})
        target_link_libraries(${_logical_name} ${PYTHON_LIBRARY})
        target_link_libraries(${_logical_name} ${EXTRA_LINK_LIBRARIES})
        set_target_properties(${_logical_name} PROPERTIES PREFIX "" OUTPUT_NAME ${_child_module_name})

        if (MINGW)
            target_compile_definitions(${_logical_name} PRIVATE _hypot=hypot)
        else()
            # SIP v5+ redefines access to protected variables.
            target_compile_definitions(${_logical_name} PRIVATE SIP_PROTECTED_IS_PUBLIC)
            target_compile_definitions(${_logical_name} PRIVATE protected=public)
        endif ()

        if (WIN32)
            SET_TARGET_PROPERTIES(${_logical_name} PROPERTIES SUFFIX ".pyd")
        ENDIF ()

        install(TARGETS ${_logical_name} DESTINATION "${PYTHON_SITE_PACKAGES_INSTALL_DIR}/${_parent_module_path}")
    endmacro()
endif()
