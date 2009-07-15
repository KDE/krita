include_directories( ${KORESOURCES_INCLUDES} 
                     ${CMAKE_SOURCE_DIR}/libs/kobase
)

set(kocvsimport_LIB_SRCS
    KoCsvImportDialog.cpp
)

kde4_add_ui_files( kocvsimport_LIB_SRCS
    KoCsvImportDialog.ui
)

kde4_add_library(kocvsimport SHARED ${kocvsimport_LIB_SRCS})

target_link_libraries(kocvsimport
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
)

set_target_properties(kocvsimport PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kocvsimport  ${INSTALL_TARGETS_DEFAULT_ARGS})
