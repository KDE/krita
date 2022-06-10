add_subdirectory(tests)

# import

set(kritacsvimport_SOURCES
    kis_csv_import.cpp
    csv_loader.cpp
    csv_read_line.cpp
    csv_layer_record.cpp
)

kis_add_library(kritacsvimport MODULE ${kritacsvimport_SOURCES})

target_link_libraries(kritacsvimport kritaui )

install(TARGETS kritacsvimport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

# export

set(kritacsvexport_SOURCES
    kis_csv_export.cpp
    csv_saver.cpp
    csv_layer_record.cpp
)

kis_add_library(kritacsvexport MODULE ${kritacsvexport_SOURCES})

target_link_libraries(kritacsvexport kritaui kritaimpex)

install(TARGETS kritacsvexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_csv.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
