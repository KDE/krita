include_directories( ${KORESOURCES_INCLUDES} 
                     ${CMAKE_SOURCE_DIR}/libs/kobase
)

set(kocharacterselectiondialog_LIB_SRCS
    KoCharSelectDia.cpp
)

kde4_add_library(kocharacterselectiondialog SHARED ${kocharacterselectiondialog_LIB_SRCS})

target_link_libraries(kocharacterselectiondialog
    ${KDE4_KDEUI_LIBS}
)

set_target_properties(kocharacterselectiondialog PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kocharacterselectiondialog  ${INSTALL_TARGETS_DEFAULT_ARGS})
