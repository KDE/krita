set(kritaglobal_LIB_SRCS
  kis_assert.cpp
)

kde4_add_library(kritaglobal SHARED ${kritaglobal_LIB_SRCS} )

target_link_libraries(kritaglobal Qt5::Concurrent ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})
target_link_libraries(kritaglobal LINK_INTERFACE_LIBRARIES Qt5::Concurrent ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kritaglobal PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritaglobal  ${INSTALL_TARGETS_DEFAULT_ARGS})

