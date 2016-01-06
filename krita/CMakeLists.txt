project(krita)

add_definitions(
  -DTRANSLATION_DOMAIN=\"krita\"
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_FAST_CONCATENATION
)

# To avoid the confusion of an endless stream of Eigen warnings.
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-unused-local-typedefs -Wno-deprecated -Wno-deprecated-declarations)
endif ()


set(KRITA_DEFAULT_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/tests/data/)

macro(macro_add_unittest_definitions)
  add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
  add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
  add_definitions(-DFILES_DEFAULT_DATA_DIR="${KRITA_DEFAULT_TEST_DATA_DIR}")
endmacro()

#
# The reason for this mode is that the Debug mode disable inlining
#
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_KRITADEVS "-O3 -g" CACHE STRING "" FORCE)
endif()

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:16777216") # for g'mic...
#    set(CMAKE_CXX_STACK_SIZE "999999999")
endif()

configure_file(image/config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/image/config_convolution.h)
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h )

include(CheckFunctionExists)
check_function_exists(powf HAVE_POWF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

if(HAVE_OPENGL)
    message(STATUS "OpenGL found -- krita will be able  to use OpenGL for hardware accelerated rendering.")
else()
    message(STATUS "OpenGL NOT found - krita will not be able to use OpenGL for hardware accelerated rendering.")
endif()

if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
endif()

if(WIN32)
    if(MSVC)
        # C4522: 'class' : multiple assignment operators specified
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4522")
    endif()
endif()

set(KRITA_INCLUDES
    # For config-*.h
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/image

    ${CMAKE_SOURCE_DIR}/krita/libpsd
    ${CMAKE_SOURCE_DIR}/krita/image
    ${CMAKE_SOURCE_DIR}/krita/image/brushengine
    ${CMAKE_SOURCE_DIR}/krita/image/tiles
    ${CMAKE_SOURCE_DIR}/krita/libbrush
    ${CMAKE_SOURCE_DIR}/krita/libcolor
    ${CMAKE_SOURCE_DIR}/krita/ui
    ${CMAKE_SOURCE_DIR}/krita/ui/canvas
    ${CMAKE_SOURCE_DIR}/krita/ui/tool
    ${CMAKE_SOURCE_DIR}/krita/ui/flake
    ${CMAKE_SOURCE_DIR}/krita/ui/widgets

    # For generated files from .ui
    ${CMAKE_BINARY_DIR}/krita/ui

    ${PIGMENT_INCLUDES}
    ${BASICFLAKES_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/libs/widgets/resources
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
    ${CMAKE_SOURCE_DIR}/libs/widgetutils

    ${EIGEN3_INCLUDE_DIR}
    ${Vc_INCLUDE_DIR}

    # 'Export' this for use by filters
    CACHE INTERNAL ""
)

include_directories(${KRITA_INCLUDES})

add_subdirectory( libpsd )
add_subdirectory( image )
add_subdirectory( libbrush )
add_subdirectory( libcolor )
add_subdirectory( ui )
add_subdirectory( dtd )
add_subdirectory( plugins )
add_subdirectory( data )
add_subdirectory( integration )
add_subdirectory( pics/app )

if (NOT WIN32)
    add_subdirectory( benchmarks )
endif ()

set(krita_SRCS main.cc)

if (NOT APPLE)
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/*-apps-calligrakrita.png")
    ecm_add_app_icon(krita_SRCS ICONS ${ICONS_SRCS})
endif()

# separate listing, both used by Krita and KritaSketch
set(krita_QRCS
    ${CMAKE_SOURCE_DIR}/krita/krita.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-dark/breeze-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-light/breeze-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layerbox/layerbox-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layerbox/svg/layerbox-svg-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-light/misc-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-dark/misc-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tools/16/tools-16-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tools/SVG/16/tools-svg-16-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tool_transform/tool-transform-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/svg/svg-icons.qrc
    ${CMAKE_SOURCE_DIR}/libs/flake/flake.qrc
    ${CMAKE_SOURCE_DIR}/libs/widgets/kritawidgets.qrc
    ${CMAKE_SOURCE_DIR}/pics/icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/aboutdata/aboutdata.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/shaders/shaders.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/cursors/cursors.qrc
)

qt5_add_resources(krita_SRCS ${krita_QRCS})

add_executable(krita ${krita_SRCS})
target_link_libraries(krita 
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
    target_link_libraries(krita
                          KF5::Crash
    )
endif()

if (APPLE)
    set_target_properties(krita PROPERTIES INSTALL_RPATH "@loader_path/../../../../lib;@loader_path/../lib;@loader_path/../Frameworks;@executable_path/../lib;@executable_path/../Frameworks")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita")
endif ()

install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  krita.rc DESTINATION ${DATA_INSTALL_DIR}/krita)
install(FILES  krita.action kritamenu.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
install(FILES  krita.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/ )

install(DIRECTORY DESTINATION ${DATA_INSTALL_DIR}/krita/shortcuts)


if (NOT APPLE AND HAVE_OPENGL AND Qt5Quick_FOUND)
   add_subdirectory(sketch)
#    add_subdirectory(gemini)
endif ()
