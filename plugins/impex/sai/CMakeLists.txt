#add_subdirectory(tests)

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

if( MSVC )
    add_compile_options( /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
    add_compile_options( -march=native )
    add_compile_options( -Wall )
    add_compile_options( -Wextra )
endif()


include_directories(3rdparty/libsai/include)

set(kritasaiimport_SOURCES
    kis_sai_import.cpp
    kis_sai_converter.cpp
    3rdparty/libsai/source/sai.cpp
)

add_library(kritasaiimport MODULE ${kritasaiimport_SOURCES})

target_link_libraries(kritasaiimport kritaui)

install(TARGETS kritasaiimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS krita_sai.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
