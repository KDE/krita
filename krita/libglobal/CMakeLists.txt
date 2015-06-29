set(libglobal_LIB_SRCS
  kis_assert.cpp
)

kde4_add_library(libglobal SHARED ${libglobal_LIB_SRCS} )

target_link_libraries(libglobal ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})
target_link_libraries(libglobal LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(libglobal PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS libglobal  ${INSTALL_TARGETS_DEFAULT_ARGS})

