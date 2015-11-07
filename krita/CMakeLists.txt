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

kde_enable_exceptions()

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
    if (USE_BREAKPAD)
        include_directories(${CMAKE_SOURCE_DIR}/3rdparty/google-breakpad/src)
    endif ()
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:16777216") # for g'mic...
#    set(CMAKE_CXX_STACK_SIZE "999999999")
endif()

macro_optional_find_package(FFTW3)
macro_log_feature(FFTW3_FOUND "FFTW3" "A fast, free C FFT library" "http://www.fftw.org/" FALSE "" "Required by the Krita for fast convolution operators")
macro_bool_to_01(FFTW3_FOUND HAVE_FFTW3)
configure_file(image/config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/image/config_convolution.h)

include(CheckFunctionExists)

if(HAVE_OPENGL)
    message(STATUS "OpenGL found -- krita will be able  to use OpenGL for hardware accelerated rendering.")
else()
    message(STATUS "OpenGL NOT found - krita will not be able to use OpenGL for hardware accelerated rendering.")
endif()

macro_optional_find_package(OCIO)
macro_log_feature(OCIO_FOUND "OCIO" "The OpenColorIO Library" "http://www.opencolorio.org" FALSE "" "Required by the Krita LUT docker")
macro_bool_to_01(OCIO_FOUND HAVE_OCIO)
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h )

if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
endif()

check_function_exists(powf HAVE_POWF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(${OPENEXR_INCLUDE_DIR})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
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

    ${CMAKE_SOURCE_DIR}/krita/libglobal
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

if(HAVE_OPENGL)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

add_subdirectory( libglobal )
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

if(WIN32 AND USE_BREAKPAD)
    set(krita_SRCS ${krita_SRCS} kis_crash_handler.cpp)
    set(BREAKPAD_LIBS breakpad)
endif()

if (NOT APPLE)
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/*-apps-calligrakrita.png")
    ecm_add_app_icon(krita_SRCS ICONS ${ICONS_SRCS})
endif()

qt5_add_resources(krita_SRCS krita.qrc 
                             pics/Breeze-dark/breeze-dark-icons.qrc
                             pics/Breeze-light/breeze-light-icons.qrc
                             pics/layerbox/layerbox-icons.qrc
                             pics/layerbox/svg/layerbox-svg-icons.qrc
                             pics/misc-light/misc-light-icons.qrc
                             pics/misc-dark/misc-dark-icons.qrc
                             pics/tools/16/tools-16-icons.qrc
                             pics/tools/SVG/16/tools-svg-16-icons.qrc
                             pics/tool_transform/tool-transform-icons.qrc
                             pics/svg/svg-icons.qrc
                             ${CMAKE_SOURCE_DIR}/libs/flake/flake.qrc
                             ${CMAKE_SOURCE_DIR}/libs/widgets/kritawidgets.qrc
                             ${CMAKE_SOURCE_DIR}/pics/icons.qrc
                             data/aboutdata/aboutdata.qrc
                             data/shaders/shaders.qrc  
)

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
                      Qt5::Concurrent
                      ${BREAKPAD_LIBS})

if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set_target_properties(krita PROPERTIES INSTALL_RPATH "@loader_path/../../../../lib;@loader_path/../lib;@loader_path/../Frameworks;@executable_path/../lib;@executable_path/../Frameworks")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita")
endif ()

install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  krita.rc DESTINATION ${DATA_INSTALL_DIR}/krita)
install(FILES  krita.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
install(FILES  krita.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/ )

if (NOT APPLE AND HAVE_OPENGL AND Qt5Quick_FOUND)
   add_subdirectory(sketch)
#    add_subdirectory(gemini)
endif ()
