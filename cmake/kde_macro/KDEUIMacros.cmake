# SPDX-FileCopyrightText: 2006-2009 Alexander Neundorf <neundorf@kde.org>
# SPDX-FileCopyrightText: 2006, 2007 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

get_filename_component(KDE4UI_MODULE_DIR  ${CMAKE_CURRENT_LIST_FILE} PATH)

# TODO: These macros belong with the framework they relate to.

#create the implementation files from the ui files and add them to the list of sources
#usage: KDE4_ADD_UI_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

      # we need to run uic and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE4_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -P ${KDE4UI_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )
      list(APPEND ${_sources} ${_header})
   endforeach (_current_FILE)
endmacro (KDE4_ADD_UI_FILES)


macro (KDE4_UPDATE_ICONCACHE)
    # Update mtime of hicolor icon theme dir.
    # We don't always have touch command (e.g. on Windows), so instead create
    #  and delete a temporary file in the theme dir.
   install(CODE "
    set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
    if (NOT DESTDIR_VALUE)
        file(WRITE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\" \"update\")
        file(REMOVE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\")
    endif (NOT DESTDIR_VALUE)
    ")
endmacro (KDE4_UPDATE_ICONCACHE)

# a "map" of short type names to the directories
# unknown names should give empty results
# KDE 3 compatibility
set(_KDE4_ICON_GROUP_mime       "mimetypes")
set(_KDE4_ICON_GROUP_filesys    "places")
set(_KDE4_ICON_GROUP_device     "devices")
set(_KDE4_ICON_GROUP_app        "apps")
set(_KDE4_ICON_GROUP_action     "actions")
# KDE 4 / icon naming specification compatibility
set(_KDE4_ICON_GROUP_mimetypes  "mimetypes")
set(_KDE4_ICON_GROUP_places     "places")
set(_KDE4_ICON_GROUP_devices    "devices")
set(_KDE4_ICON_GROUP_apps       "apps")
set(_KDE4_ICON_GROUP_actions    "actions")
set(_KDE4_ICON_GROUP_categories "categories")
set(_KDE4_ICON_GROUP_status     "status")
set(_KDE4_ICON_GROUP_emblems    "emblems")
set(_KDE4_ICON_GROUP_emotes     "emotes")
set(_KDE4_ICON_GROUP_animations "animations")
set(_KDE4_ICON_GROUP_intl       "intl")

# a "map" of short theme names to the theme directory
set(_KDE4_ICON_THEME_ox "oxygen")
set(_KDE4_ICON_THEME_cr "crystalsvg")
set(_KDE4_ICON_THEME_lo "locolor")
set(_KDE4_ICON_THEME_hi "hicolor")


# only used internally by KDE4_INSTALL_ICONS
macro (_KDE4_ADD_ICON_INSTALL_RULE _install_SCRIPT _install_PATH _group _orig_NAME _install_NAME _l10n_SUBDIR)

   # if the string doesn't match the pattern, the result is the full string, so all three have the same content
   if (NOT ${_group} STREQUAL ${_install_NAME} )
      set(_icon_GROUP  ${_KDE4_ICON_GROUP_${_group}})
      if(NOT _icon_GROUP)
         set(_icon_GROUP "actions")
      endif(NOT _icon_GROUP)
#      message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name} l10n: ${_l10n_SUBDIR}")
      install(FILES ${_orig_NAME} DESTINATION ${_install_PATH}/${_icon_GROUP}/${_l10n_SUBDIR}/ RENAME ${_install_NAME} )
   endif (NOT ${_group} STREQUAL ${_install_NAME} )

endmacro (_KDE4_ADD_ICON_INSTALL_RULE)


macro (KDE4_INSTALL_ICONS _defaultpath )

   # the l10n-subdir if language given as second argument (localized icon)
   set(_lang ${ARGV1})
   if(_lang)
      set(_l10n_SUBDIR l10n/${_lang})
   else(_lang)
      set(_l10n_SUBDIR ".")
   endif(_lang)

   # first the png icons
   file(GLOB _icons *.png)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                    ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                    ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # mng icons
   file(GLOB _icons *.mng)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # and now the svg icons
   file(GLOB _icons *.svgz)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" _dummy "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_group "${CMAKE_MATCH_2}")
      set(_name  "${CMAKE_MATCH_3}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
          _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                            ${_defaultpath}/${_theme_GROUP}/scalable
                            ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   kde4_update_iconcache()

endmacro (KDE4_INSTALL_ICONS)


macro (KDE4_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${ARGN})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KDE4_MAKEKDEWIDGETS_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KDE4_MAKEKDEWIDGETS_DEP})

      # create moc file
      qt4_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KDE4_ADD_WIDGET_FILES)

# adds application icon to target source list
# 'appsources' - the sources of the application
# 'pngfiles' - specifies the list of icon files
# example: KDE4_ADD_WIN32_APP_ICON(myapp_SRCS "pics/cr16-myapp.png;pics/cr32-myapp.png")
macro (KDE4_ADD_WIN32_APP_ICON appsources)
    message(STATUS "KDE4_ADD_WIN32_APP_ICON() is deprecated, use KDE4_ADD_APP_ICON() instead")
    if (WIN32)
        if(NOT WINCE)
        find_program(PNG2ICO_EXECUTABLE NAMES png2ico)
        else(NOT WINCE)
        find_program(PNG2ICO_EXECUTABLE NAMES png2ico PATHS ${HOST_BINDIR} NO_DEFAULT_PATH )
        endif(NOT WINCE)
        string(REPLACE _SRCS "" appname ${appsources})
        if (PNG2ICO_EXECUTABLE)
            set (_outfilename ${CMAKE_CURRENT_BINARY_DIR}/${appname})

            # png2ico is found by the above find_program
#            message("png2ico ${_outfilename}.ico ${ARGN}")
            exec_program(png2ico ARGS ${_outfilename}.ico ${ARGN})

            # now make rc file for adding it to the sources
            file(WRITE ${_outfilename}.rc "IDI_ICON1        ICON        DISCARDABLE    \"${_outfilename}.ico\"\n")
                list(APPEND ${appsources} ${CMAKE_CURRENT_BINARY_DIR}/${appname}.rc)
        endif(PNG2ICO_EXECUTABLE)
    endif(WIN32)
endmacro (KDE4_ADD_WIN32_APP_ICON)

# adds application icon to target source list
# for detailed documentation see the top of FindKDE4Internal.cmake
macro (KDE4_ADD_APP_ICON appsources pattern)
    set (_outfilename ${CMAKE_CURRENT_BINARY_DIR}/${appsources})

    if (WIN32)
        if(NOT WINCE)
        find_program(PNG2ICO_EXECUTABLE NAMES png2ico)
        else(NOT WINCE)
        find_program(PNG2ICO_EXECUTABLE NAMES png2ico PATHS ${HOST_BINDIR} NO_DEFAULT_PATH )
        endif(NOT WINCE)
        if (PNG2ICO_EXECUTABLE)
            string(REPLACE "*" "(.*)" pattern_rx "${pattern}")
            file(GLOB files  "${pattern}")
            foreach (it ${files})
                string(REGEX REPLACE "${pattern_rx}" "\\1" fn "${it}")
                if (fn MATCHES ".*16.*" )
                    list (APPEND _icons ${it})
                endif (fn MATCHES ".*16.*")
                if (fn MATCHES ".*32.*" )
                    list (APPEND _icons ${it})
                endif (fn MATCHES ".*32.*")
                if (fn MATCHES ".*48.*" )
                    list (APPEND _icons ${it})
                endif (fn MATCHES ".*48.*")
                if (fn MATCHES ".*64.*" )
                    list (APPEND _icons ${it})
                endif (fn MATCHES ".*64.*")
                if (fn MATCHES ".*128.*" )
                    list (APPEND _icons ${it})
                endif (fn MATCHES ".*128.*")
            endforeach (it)
            if (_icons)
                add_custom_command(OUTPUT ${_outfilename}.ico ${_outfilename}.rc
                                   COMMAND ${PNG2ICO_EXECUTABLE} ARGS --rcfile ${_outfilename}.rc ${_outfilename}.ico ${_icons}
                                   DEPENDS ${PNG2ICO_EXECUTABLE} ${_icons}
                                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                                  )
                    list(APPEND ${appsources} ${_outfilename}.rc)
            else(_icons)
                message(STATUS "Unable to find a related icon that matches pattern ${pattern} for variable ${appsources} - application will not have an application icon!")
            endif(_icons)
        else(PNG2ICO_EXECUTABLE)
            message(STATUS "Unable to find the png2ico utility - application will not have an application icon!")
        endif(PNG2ICO_EXECUTABLE)
    endif(WIN32)
    if (APPLE)
        # first convert image to a tiff using the Mac OS X "sips" utility,
        # then use tiff2icns to convert to an icon
        find_program(SIPS_EXECUTABLE NAMES sips)
        find_program(TIFF2ICNS_EXECUTABLE NAMES tiff2icns)
        if (SIPS_EXECUTABLE AND TIFF2ICNS_EXECUTABLE)
            file(GLOB_RECURSE files  "${pattern}")
            # we can only test for the 128-icon like that - we don't use patterns anymore
            foreach (it ${files})
                if (it MATCHES ".*128.*" )
                    set (_icon ${it})
                endif (it MATCHES ".*128.*")
            endforeach (it)

            if (_icon)

                # first, get the basename of our app icon
                add_custom_command(OUTPUT ${_outfilename}.icns ${outfilename}.tiff
                                   COMMAND ${SIPS_EXECUTABLE} -s format tiff ${_icon} --out ${outfilename}.tiff
                                   COMMAND ${TIFF2ICNS_EXECUTABLE} ${outfilename}.tiff ${_outfilename}.icns
                                   DEPENDS ${_icon}
                                   )

                # This will register the icon into the bundle
                set(MACOSX_BUNDLE_ICON_FILE ${appsources}.icns)

                # Append the icns file to the sources list so it will be a dependency to the
                # main target
                list(APPEND ${appsources} ${_outfilename}.icns)

                # Install the icon into the Resources dir in the bundle
                set_source_files_properties(${_outfilename}.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

            else(_icon)
                # TODO - try to scale a non-128 icon...? Try to convert an SVG on the fly?
                message(STATUS "Unable to find an 128x128 icon that matches pattern ${pattern} for variable ${appsources} - application will not have an application icon!")
            endif(_icon)

        else(SIPS_EXECUTABLE AND TIFF2ICNS_EXECUTABLE)
            message(STATUS "Unable to find the sips and tiff2icns utilities - application will not have an application icon!")
        endif(SIPS_EXECUTABLE AND TIFF2ICNS_EXECUTABLE)
     endif(APPLE)
endmacro (KDE4_ADD_APP_ICON)

