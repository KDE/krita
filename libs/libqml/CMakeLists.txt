add_subdirectory(plugins)

set(kritaqml_SRCS
    DocumentManager.cpp
    DocumentListModel.cpp
    KisSelectionExtras.cpp
    RecentFileManager.cpp
    ProgressProxy.cpp
    PropertyContainer.cpp
    Settings.cpp
    VirtualKeyboardController.cpp
    Theme.cpp
    QmlGlobalEngine.cpp
    KisSketchView.cpp
)

qt6_add_resources(kritaqml_SRCS qml/qml.qrc)

kis_add_library(kritaqml SHARED ${kritaqml_SRCS})
generate_export_header(kritaqml BASE_NAME krita_sketch)
set_target_properties(kritaqml
    PROPERTIES VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

target_link_libraries(kritaqml
    Qt6::Quick
    Qt6::Gui
    Qt6::Core
    kritawidgets
    kritaui
)

include(GNUInstallDirs)
install(TARGETS kritaqml ${INSTALL_TARGETS_DEFAULT_ARGS})
install(DIRECTORY qmlthemes DESTINATION ${DATA_INSTALL_DIR}/krita)

