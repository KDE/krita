project(krita)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${Vc_INCLUDE_DIR}
)

add_subdirectory( dtd )
add_subdirectory( data )
add_subdirectory( integration )
add_subdirectory( pics/app )

if (WIN32 AND USE_BREAKPAD)
    add_subdirectory( crashreporter )
    include_directories(${CMAKE_SOURCE_DIR}/3rdparty/google-breakpad/src)
endif ()

set(krita_SRCS main.cc)

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

if(WIN32 AND USE_BREAKPAD)
    set(krita_SRCS ${krita_SRCS} kis_crash_handler.cpp)
    find_library(BREAKPAD_LIBS breakpad)
    if(NOT BREAKPAD_LIBS)
        message(FATAL_ERROR "Couldn't find Google Breakpad library!")
    endif()
endif()


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

add_executable(krita ${krita_SRCS})
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
                      ${BREAKPAD_LIBS}
                      Qt5::Concurrent)
if(HAVE_KCRASH)
    target_link_libraries(krita
        PRIVATE
                          KF5::Crash
    )
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
install(PROGRAMS krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  krita.action kritamenu.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
install(FILES  krita.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/ )

install(DIRECTORY DESTINATION ${DATA_INSTALL_DIR}/krita/shortcuts)

