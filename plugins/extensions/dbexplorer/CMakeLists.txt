set(kritadbexplorer_SOURCES
    DbExplorer.cpp
    DlgDbExplorer.cpp
    TableModel.cpp
)

ki18n_wrap_ui(kritadbexplorer_SOURCES WdgDbExplorer.ui )

kis_add_library(kritadbexplorer MODULE ${kritadbexplorer_SOURCES})

target_link_libraries(kritadbexplorer
    PRIVATE
        kritaresources
        kritaresourcewidgets
        kritaui
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt${QT_MAJOR_VERSION}::Sql
)
install(TARGETS kritadbexplorer DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES dbexplorer.xmlgui DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install(FILES dbexplorer.action DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
