include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${Vc_INCLUDE_DIR}
)

add_subdirectory( dtd )
add_subdirectory( data )
add_subdirectory( integration )
# Install the application icons following the freedesktop icon theme spec
add_subdirectory( "pics/branding/${BRANDING}" )

if (ANDROID)
    include_directories (${Qt5AndroidExtras_INCLUDE_DIRS})
endif()

set(krita_SRCS main.cc)

# Set the application icon on the application
if (NOT APPLE)
	file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/*-apps-krita.png")
else()
	set(ICON_SRCS
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/16-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/32-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/48-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/128-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/256-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/512-apps-krita.png"
	     "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/1024-apps-krita.png"
	)
endif()

if (WIN32)
    set(krita_windows_stub_ICONS_SRCS "")
    ecm_add_app_icon(krita_windows_stub_ICONS_SRCS ICONS ${ICON_SRCS} OUTFILE_BASENAME krita)

    # This is used to generate an ICO file for the file icon, We don't actually
    # want to embed this icon into an EXE.
	file(GLOB krafile_ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/*-mimetypes-application-x-krita.png")
    set(krafile_dummy_ICONS_SRCS "")
    ecm_add_app_icon(krafile_dummy_ICONS_SRCS ICONS ${krafile_ICON_SRCS} OUTFILE_BASENAME kritafile)
    add_library(krafile_dummy_obj OBJECT ${krafile_dummy_ICONS_SRCS})

    # These icons are used to replace those in the shell extension package.
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/krita.ico
            ${CMAKE_CURRENT_BINARY_DIR}/kritafile.ico
        DESTINATION ${CMAKE_INSTALL_PREFIX}/installer
    )
else()
    ecm_add_app_icon(krita_SRCS ICONS ${ICON_SRCS})
endif()

# Install the mimetype icons
ecm_install_icons(ICONS
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/16-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/22-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/32-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/48-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/64-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/128-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/256-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/512-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/1024-mimetypes-application-x-krita.png"
    DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

# separate listing, both used by Krita and KritaSketch
set(krita_QRCS
    ${CMAKE_SOURCE_DIR}/krita/krita.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-dark/breeze-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-light/breeze-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layerbox/layerbox-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layers/layers-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-light/misc-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-dark/misc-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/paintops/paintops-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tools/SVG/16/tools-svg-16-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tool_transform/tool-transform-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/svg/svg-icons.qrc
    ${CMAKE_SOURCE_DIR}/libs/flake/flake.qrc
    ${CMAKE_SOURCE_DIR}/libs/widgets/kritawidgets.qrc
    ${CMAKE_SOURCE_DIR}/pics/icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/aboutdata/aboutdata.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/shaders/shaders.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/cursors/cursors.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/gamutmasks/gamutmasks.qrc
    ${CMAKE_SOURCE_DIR}/plugins/paintops/defaultpresets/defaultpresets.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/branding/${BRANDING}/branding.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/splash/splash.qrc
    CACHE INTERNAL "krita_QRCS"
)

qt5_add_resources(krita_SRCS ${krita_QRCS})

if (ANDROID)
    add_library(krita SHARED ${krita_SRCS})
    target_link_libraries(krita PRIVATE Qt5::AndroidExtras)
elseif (WIN32)
    # Build the main implementation into a DLL to be called by a stub EXE.
    add_library(krita SHARED ${krita_SRCS})
    set_target_properties(krita PROPERTIES PREFIX "")
else()
    add_executable(krita ${krita_SRCS})
endif()
target_link_libraries(krita
    PRIVATE
                      kritaui
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Xml
                      Qt5::Network
                      Qt5::PrintSupport
                      Qt5::Svg
                      Qt5::Concurrent)
if(HAVE_KCRASH)
    target_link_libraries(krita PRIVATE KF5::Crash)
endif()

if (APPLE)
    set_target_properties(krita PROPERTIES INSTALL_RPATH "@loader_path/../../../../lib;@loader_path/../lib;
        @loader_path/../Frameworks;@executable_path/../../../../lib;@executable_path/../lib;@executable_path/../Frameworks")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_ICON_FILE "krita_SRCS.icns")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_VERSION ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_COPYRIGHT "GNU Public License, V2 or, at your option, any later version.")
endif ()

if (NOT ANDROID)
  set(KRITAVERSION_SRCS kritaversion.cpp)
  add_executable(krita_version ${KRITAVERSION_SRCS})
  target_link_libraries(krita_version
    PRIVATE
        Qt5::Core
        kritaversion
  )


  install(TARGETS krita_version ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()


install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.krita.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES  krita.action kritamenu.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
install(FILES  org.kde.krita.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

install(DIRECTORY DESTINATION ${DATA_INSTALL_DIR}/krita/shortcuts)

if (WIN32)
    configure_file(versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)

    add_executable(krita_windows_stub_exe windows_stub_main.cpp ${krita_windows_stub_ICONS_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
    target_link_libraries(krita_windows_stub_exe PRIVATE krita Qt5::WinMain)
    set_target_properties(krita_windows_stub_exe
        PROPERTIES
            OUTPUT_NAME "krita"
    )
    if (MSVC)
        target_link_options(krita_windows_stub_exe PRIVATE "/DEBUG:NONE")
    endif()
    install(TARGETS krita_windows_stub_exe ${INSTALL_TARGETS_DEFAULT_ARGS})

    add_executable(krita_windows_stub_com windows_stub_main.cpp ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
    target_link_libraries(krita_windows_stub_com PRIVATE krita)
    set_target_properties(krita_windows_stub_com
        PROPERTIES
            WIN32_EXECUTABLE false
            OUTPUT_NAME "krita"
            SUFFIX ".com"
    )
    if (MSVC)
        target_link_options(krita_windows_stub_com PRIVATE "/DEBUG:NONE")
    endif()
    install(TARGETS krita_windows_stub_com ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (WIN32)
