add_subdirectory(tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenJPEG_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenJPEG_CFLAGS}")
add_definitions(${OpenJPEG_DEFINITIONS})

include_directories(SYSTEM ${OpenJPEG_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    jp2_converter.cc
)

set(kritajp2import_SOURCES
    jp2_import.cc
    ${libkritaconverter_LIB_SRCS}
)

add_library(kritajp2import MODULE ${kritajp2import_SOURCES})

target_link_libraries(kritajp2import kritaui ${OpenJPEG_LIBRARIES} )

install(TARGETS kritajp2import DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_jp2.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
