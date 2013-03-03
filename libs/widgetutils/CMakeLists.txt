include_directories(${KDE4_INCLUDES})

set(kowidgetutils_LIB_SRCS
    KoGroupButton.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
)


kde4_add_library(kowidgetutils SHARED ${kowidgetutils_LIB_SRCS})

target_link_libraries(kowidgetutils ${KDE4_KDEUI_LIBS})
target_link_libraries(kowidgetutils LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

set_target_properties(kowidgetutils
    PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kowidgetutils ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    KoGroupButton.h
    KoProgressBar.h
    KoProgressProxy.h
    KoProgressUpdater.h
    KoUpdater.h
    kowidgetutils_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)
