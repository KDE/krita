project(krita)


# 
# The reason for this mode is that the Debug mode disable inlining
#
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_KRITADEVS "-O2 -g" CACHE STRING "" FORCE)
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif(MSVC)

include(CheckFunctionExists)

if (NOT WIN32)
macro_optional_find_package(GLEW)
macro_bool_to_01(GLEW_FOUND HAVE_GLEW)
configure_file(config-glew.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-glew.h )
macro_log_feature(HAVE_GLEW "GLEW" "The OpenGL Extension Wrangler Library" "http://glew.sourceforge.net" FALSE "" "Required by the Krita shader filter plugin")

if(HAVE_GLEW)
        message(STATUS "GLEW found!")
else(HAVE_GLEW)
        message(STATUS "GLEW NOT found - krita will not be able to use OpenGL shaders")
endif(HAVE_GLEW)
if(HAVE_OPENGL)
    message(STATUS "OpenGL found -- krita will be able  to use OpenGL for hardware accelerated rendering.")
else(HAVE_OPENGL)
    message(STATUS "OpenGL NOT found - krita will not be able to use OpenGL for hardware accelerated rendering.")
endif(HAVE_OPENGL)
endif (NOT WIN32)

macro_optional_find_package(OCIO)
macro_log_feature(OCIO_FOUND "OCIO" "The OpenColorIO Library" "http://www.opencolorio.org" FALSE "" "Required by the Krita LUT docker")
macro_bool_to_01(OCIO_FOUND HAVE_OCIO)
configure_file(config-ocio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocio.h )

macro_optional_find_package(OIIO)
macro_log_feature(OIIO_FOUND "OIIO" "The OpenImageIO Lbirary" "https://sites.google.com/site/openimageio/" FALSE "" "Required by the OIIO file import/export filter")
macro_bool_to_01(OIIO_FOUND HAVE_OIIO)
configure_file(config-oiio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-oiio.h )


if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
endif(UNIX)

check_function_exists(powf HAVE_POWF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(${OPENEXR_INCLUDE_DIR})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif(OPENEXR_FOUND)

if(WIN32)
    if(MSVC)
        # C4522: 'class' : multiple assignment operators specified
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4522")
    endif(MSVC)
endif(WIN32)

set(KRITA_INCLUDES
    # For config-*.h
    ${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_SOURCE_DIR}/krita/image
    ${CMAKE_SOURCE_DIR}/krita/image/brushengine
    ${CMAKE_SOURCE_DIR}/krita/image/tiles
    ${CMAKE_SOURCE_DIR}/krita/ui
    ${CMAKE_SOURCE_DIR}/krita/ui/canvas
    ${CMAKE_SOURCE_DIR}/krita/ui/tool
    ${CMAKE_SOURCE_DIR}/krita/ui/flake
    ${CMAKE_SOURCE_DIR}/krita/ui/widgets

    # For generated files from .ui
    ${CMAKE_BINARY_DIR}/krita/ui
    
    ${PIGMENT_INCLUDES}
    ${KOTEXT_INCLUDES}
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/resources
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
    
    ${EIGEN2_INCLUDE_DIR}
    
    # 'Export' this for use by filters
    CACHE INTERNAL ""
)

include_directories(${KRITA_INCLUDES})

if(HAVE_OPENGL)
    include_directories(${OPENGL_INCLUDE_DIR})
    if(HAVE_GLEW)
        include_directories(${GLEW_INCLUDE_DIR})
    endif(HAVE_GLEW)
endif(HAVE_OPENGL)

add_subdirectory( image )
add_subdirectory( ui )
add_subdirectory( dtd )
add_subdirectory( plugins )
add_subdirectory( data )
add_subdirectory( pics )
add_subdirectory( benchmarks )

set(kritapart_PART_SRCS krita_part_init.cc)
kde4_add_plugin(kritapart ${kritapart_PART_SRCS})
target_link_libraries(kritapart kritaui)
install(TARGETS kritapart DESTINATION ${PLUGIN_INSTALL_DIR})

set(krita_KDEINIT_SRCS main.cc)
kde4_add_app_icon(krita_KDEINIT_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/krita.png")
kde4_add_kdeinit_executable(krita ${krita_KDEINIT_SRCS})

if (Q_WS_MAC)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.calligra.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_krita kritaui)
target_link_libraries(krita ${QT_QTCORE_LIBRARY} kdeinit_krita)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_package(OpenCTL)
    find_package(OpenShiva)
    find_package(QtShiva )
    
    if(NOT ${OPENCTL_VERSION} VERSION_GREATER 0.9.15.1 )
        # Workaround for the following issues:
        # http://bugs.kde.org/274675 http://bugs.kde.org/248051
        # http://redmine.opengtl.org/issues/72
        message(status "Found OpenGTL < 0.9.15.1, on a 64 bits system, krita will be forced linked to the libraries to make sure opengtl symbols are available")
        if(HAVE_OPENSHIVA AND HAVE_QTSHIVA)
            target_link_libraries(krita ${OPENSHIVA_LDFLAGS})
        endif()

        if(OPENCTL_FOUND AND OPENEXR_FOUND)
            target_link_libraries(krita ${OPENCTL_LDFLAGS})
        endif()
    endif()
endif()

install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install( PROGRAMS  krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kritapart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita.rc DESTINATION ${DATA_INSTALL_DIR}/krita)



