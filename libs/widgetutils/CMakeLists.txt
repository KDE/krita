include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

add_subdirectory(tests)

set(kowidgetutils_LIB_SRCS
    KoGroupButton.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
    KoProperties.cpp
    KoSizeGroup.cpp
    KoSizeGroupPrivate.cpp
)


kde4_add_library(kowidgetutils SHARED ${kowidgetutils_LIB_SRCS})

# KDE is only used for i18n
target_link_libraries(kowidgetutils ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})
target_link_libraries(kowidgetutils LINK_INTERFACE_LIBRARIES ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})

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
    KoProperties.h
    KoSizeGroup.h
    kowidgetutils_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/calligra
    COMPONENT Devel
)
