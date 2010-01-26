project(krita)


# turn on QImage as buffer for QPainter based canvas
add_definitions(-DINDEPENDENT_CANVAS)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_KRITADEVS "-O2 -g" CACHE STRING "" FORCE)
endif(CMAKE_COMPILER_IS_GNUCXX)

include(CheckFunctionExists)

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
    ${KOBASE_INCLUDES}
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
add_subdirectory( shape )
add_subdirectory( dtd )
add_subdirectory( plugins )
add_subdirectory( data )
add_subdirectory( pics )
add_subdirectory( benchmarks )

set(kritapart_PART_SRCS krita_part_init.cc)
kde4_add_plugin(kritapart WITH_PREFIX ${kritapart_PART_SRCS})
target_link_libraries(kritapart kritaui)
install(TARGETS kritapart DESTINATION ${PLUGIN_INSTALL_DIR})

set(krita_KDEINIT_SRCS main.cc)
kde4_add_app_icon(krita_KDEINIT_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/krita.png")
kde4_add_kdeinit_executable(krita ${krita_KDEINIT_SRCS})

if (Q_WS_MAC)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.koffice.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_krita kritaui)
target_link_libraries(krita ${QT_QTCORE_LIBRARY} kdeinit_krita)

install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kdeinit_krita ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES  krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kritapart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita.rc krita_readonly.rc  DESTINATION ${DATA_INSTALL_DIR}/krita)



