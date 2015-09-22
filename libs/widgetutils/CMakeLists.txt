add_subdirectory(tests)

set(kowidgetutils_LIB_SRCS
    KoGroupButton.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
    KoProperties.cpp
    KoFileDialog.cpp
)


add_library(kowidgetutils SHARED ${kowidgetutils_LIB_SRCS})
generate_export_header(kowidgetutils BASE_NAME kowidgetutils)

target_link_libraries(kowidgetutils
    PUBLIC
        Qt5::Widgets
        Qt5::Gui
        Qt5::Xml
        Qt5::Core
    PRIVATE
        KF5::I18n
        KF5::ConfigCore
)

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
    KoFileDialog.h
    ${CMAKE_CURRENT_BINARY_DIR}/kowidgetutils_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/calligra
    COMPONENT Devel
)
