include_directories( ${KORESOURCES_INCLUDES} 
                     ${CMAKE_SOURCE_DIR}/libs/kobase
)

set(koaspectbutton_LIB_SRCS
    KoAspectButton.cpp
    KoImageResource.cpp
)

kde4_add_library(koaspectbutton SHARED ${koaspectbutton_LIB_SRCS})

target_link_libraries(koaspectbutton
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
)

set_target_properties(koaspectbutton PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koaspectbutton  ${INSTALL_TARGETS_DEFAULT_ARGS})
