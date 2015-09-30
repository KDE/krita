add_subdirectory(tests)

set(kritawidgetutils_LIB_SRCS
    KoGroupButton.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
    KoProperties.cpp
    KoFileDialog.cpp
)


add_library(kritawidgetutils SHARED ${kritawidgetutils_LIB_SRCS})
generate_export_header(kritawidgetutils BASE_NAME kritawidgetutils)

target_link_libraries(kritawidgetutils
    PUBLIC
        Qt5::Widgets
        Qt5::Gui
        Qt5::Xml
        Qt5::Core
    PRIVATE
        KF5::I18n
        KF5::ConfigCore
)

set_target_properties(kritawidgetutils
    PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritawidgetutils ${INSTALL_TARGETS_DEFAULT_ARGS})
