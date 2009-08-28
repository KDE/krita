include_directories( ${KORESOURCES_INCLUDES} 
                     ${CMAKE_SOURCE_DIR}/libs/kobase
)

set(kolinestyleselector_LIB_SRCS
    KoLineStyleSelector.cpp
)

kde4_add_library(kolinestyleselector SHARED ${kolinestyleselector_LIB_SRCS})

target_link_libraries(kolinestyleselector
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)

set_target_properties(kolinestyleselector PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kolinestyleselector  ${INSTALL_TARGETS_DEFAULT_ARGS})
