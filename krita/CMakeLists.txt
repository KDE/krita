project(krita)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${Vc_INCLUDE_DIR}
)

add_subdirectory( dtd )
add_subdirectory( data )
add_subdirectory( integration )
# Install the application icons following the the freedesktop icon theme spec
add_subdirectory( pics/app )

if (ANDROID)
    include_directories (${Qt5AndroidExtras_INCLUDE_DIRS})
endif()

set(krita_SRCS main.cc)

# Set the application icon on the application
if (NOT APPLE)
    file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/*-apps-calligrakrita.png")
else()
    set(ICON_SRCS
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/16-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/32-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/48-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/128-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/256-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/512-apps-calligrakrita.png"
         "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/1024-apps-calligrakrita.png"
    )
endif() 
ecm_add_app_icon(krita_SRCS ICONS ${ICON_SRCS})

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
    ${CMAKE_SOURCE_DIR}/krita/pics/layerbox/svg/layerbox-svg-icons.qrc
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
    CACHE INTERNAL "krita_QRCS"
)

qt5_add_resources(krita_SRCS ${krita_QRCS})

if (ANDROID)
    add_library(krita SHARED ${krita_SRCS})
    target_link_libraries(krita PRIVATE Qt5::AndroidExtras)
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
    set_target_properties(krita PROPERTIES INSTALL_RPATH "@loader_path/../../../../lib;@loader_path/../lib;@loader_path/../Frameworks;@executable_path/../lib;@executable_path/../Frameworks")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_ICON_FILE "krita_SRCS.icns")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_VERSION ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_COPYRIGHT "GNU Public License, V2 or, at your option, any later version.")
endif ()

install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  krita.action kritamenu.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
install(FILES  org.kde.krita.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

install(DIRECTORY DESTINATION ${DATA_INSTALL_DIR}/krita/shortcuts)

