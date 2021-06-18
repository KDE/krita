# SPDX-FileCopyrightText: 2012, 2013 Shaheed Haque <srhaque@theiet.org>
# SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
# SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
#
# SPDX-License-Identifier: BSD-2-Clause

include(CMakeParseArguments)

#
# Simple helper function to install plugin and related files
# having only a name of the plugin...
# (just to reduce syntactic noise when a lot of plugins get installed)
#
function(install_pykrita_plugin name)
    set(_options)
    set(_one_value_args)
    set(_multi_value_args PATTERNS FILE)
    cmake_parse_arguments(install_pykrita_plugin "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})
    if(NOT name)
        message(FATAL_ERROR "Plugin filename is not given")
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}.py)
        install(FILES kritapykrita_${name}.desktop DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita)
        foreach(_f ${name}.py ${name}.ui ${install_pykrita_plugin_FILE})
            if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_f})
                install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_f} DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita)
            endif()
        endforeach()
    elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${name})
        install(FILES ${name}/kritapykrita_${name}.desktop DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita)
        install(
            DIRECTORY ${name}
            DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita
            FILES_MATCHING
                PATTERN "*.py"
                PATTERN "*.ui"
                PATTERN "*.txt"
                PATTERN "*.csv"
                PATTERN "*.html"
                PATTERN "__pycache__*" EXCLUDE
                PATTERN "tests*" EXCLUDE
           )
        # TODO Is there any way to form a long PATTERN options string
        # and use it in a single install() call?
        # NOTE Install specified patterns one-by-one...
        foreach(_pattern ${install_pykrita_plugin_PATTERNS})
            install(
                DIRECTORY ${name}
                DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita
                FILES_MATCHING
                    PATTERN "${_pattern}"
                    PATTERN "__pycache__*" EXCLUDE
		    PATTERN "tests*" EXCLUDE
              )
        endforeach()
    else()
        message(FATAL_ERROR "Do not know what to do with ${name}")
    endif()
endfunction()

install_pykrita_plugin(assignprofiledialog)
install_pykrita_plugin(scripter)
install_pykrita_plugin(colorspace)
install_pykrita_plugin(documenttools)
install_pykrita_plugin(filtermanager)
install_pykrita_plugin(exportlayers)
install_pykrita_plugin(batch_exporter)
#install_pykrita_plugin(highpass)
install_pykrita_plugin(tenbrushes)
install_pykrita_plugin(tenscripts)
#install_pykrita_plugin(palette_docker)  # Needs fixing -> bug 405194
install_pykrita_plugin(quick_settings_docker)
install_pykrita_plugin(lastdocumentsdocker)
# install_pykrita_plugin(scriptdocker)
install_pykrita_plugin(comics_project_management_tools)
install_pykrita_plugin(krita_script_starter)
install_pykrita_plugin(plugin_importer)
install_pykrita_plugin(mixer_slider_docker)
install_pykrita_plugin(channels2layers)
install_pykrita_plugin(photobash_images)

# if(PYTHON_VERSION_MAJOR VERSION_EQUAL 3)
#     install_pykrita_plugin(cmake_utils)
#     install_pykrita_plugin(js_utils PATTERNS "*.json")
#     install_pykrita_plugin(expand PATTERNS "*.expand" "templates/*.tpl")
# endif()

install( FILES
    tenbrushes/tenbrushes.action
    tenscripts/tenscripts.action
    plugin_importer/plugin_importer.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)

install(
    DIRECTORY libkritapykrita
    DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita
    FILES_MATCHING
        PATTERN "*.py"
        PATTERN "__pycache__*" EXCLUDE
  )
