set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HEIF_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HEIF_CFLAGS}")
add_definitions(${HEIF_DEFINITIONS})

set(kritaheifimport_SOURCES
    HeifImport.cpp
    HeifError.cpp
)

add_library(kritaheifimport MODULE ${kritaheifimport_SOURCES})

target_link_libraries(kritaheifimport kritaui kritalibkra
    ${HEIF_LDFLAGS} ${HEIF_LIBRARIES}
)

install(TARGETS kritaheifimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaheifexport_SOURCES
    HeifExport.cpp
    HeifError.cpp
)

ki18n_wrap_ui(kritaheifexport_SOURCES WdgHeifExport.ui )

add_library(kritaheifexport MODULE ${kritaheifexport_SOURCES})

target_link_libraries(kritaheifexport kritaui kritalibkra kritaimpex ${HEIF_LDFLAGS} ${HEIF_LIBRARIES} )

install(TARGETS kritaheifexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_heif.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
