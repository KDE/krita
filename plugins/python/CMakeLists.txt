# Copyright (C) 2012, 2013 Shaheed Haque <srhaque@theiet.org>
# Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
# Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
              )
        endforeach()
    else()
        message(FATAL_ERROR "Do not know what to do with ${name}")
    endif()
endfunction()

install_pykrita_plugin(hello)
install_pykrita_plugin(assignprofiledialog)
install_pykrita_plugin(scripter)
install_pykrita_plugin(colorspace)
install_pykrita_plugin(documenttools)
install_pykrita_plugin(filtermanager)
install_pykrita_plugin(exportlayers)
#install_pykrita_plugin(highpass)
install_pykrita_plugin(tenbrushes)
install_pykrita_plugin(tenscripts)
install_pykrita_plugin(palette_docker)
install_pykrita_plugin(quick_settings_docker)
install_pykrita_plugin(lastdocumentsdocker)
# install_pykrita_plugin(scriptdocker)
install_pykrita_plugin(comics_project_management_tools)
install_pykrita_plugin(krita_script_starter)

# if(PYTHON_VERSION_MAJOR VERSION_EQUAL 3)
#     install_pykrita_plugin(cmake_utils)
#     install_pykrita_plugin(js_utils PATTERNS "*.json")
#     install_pykrita_plugin(expand PATTERNS "*.expand" "templates/*.tpl")
# endif()

install( FILES
    hello/hello.xmlgui
DESTINATION ${DATA_INSTALL_DIR}/kritaplugins/)



install( FILES
    hello/hello.action
    tenbrushes/tenbrushes.action
    tenscripts/tenscripts.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)

install(
    DIRECTORY libkritapykrita
    DESTINATION ${DATA_INSTALL_DIR}/krita/pykrita
    FILES_MATCHING
        PATTERN "*.py"
        PATTERN "__pycache__*" EXCLUDE
  )
