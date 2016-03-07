add_subdirectory(tests)

include_directories( 
${CMAKE_SOURCE_DIR}/krita/image/metadata 
${Boost_INCLUDE_DIRS}
)

# import

set(kritacsvimport_SOURCES
    kis_csv_import.cpp
    csv_loader.cpp
    csv_read_line.cpp
    csv_layer_record.cpp
)

add_library(kritacsvimport MODULE ${kritacsvimport_SOURCES})

target_link_libraries(kritacsvimport kritaui )

install(TARGETS kritacsvimport DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

# export

set(kritacsvexport_SOURCES
    kis_csv_export.cpp
    csv_saver.cpp
    csv_layer_record.cpp
)

add_library(kritacsvexport MODULE ${kritacsvexport_SOURCES})

target_link_libraries(kritacsvexport kritaui )

install(TARGETS kritacsvexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_csv.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
