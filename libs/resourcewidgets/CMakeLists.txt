add_subdirectory(dbexplorer)

set(kritaresourcewidgets_LIB_SRCS
    KisIconToolTip.cpp
    KisResourceItemChooserContextMenu.cpp
    KisResourceItemChooser.cpp
    KisResourceItemChooserSync.cpp
    KisResourceItemDelegate.cpp
    KisResourceItemListView.cpp
    KisResourceItemView.cpp
    KisResourceThumbnailPainter.cpp
    KisTagChooserWidget.cpp
    KisTagFilterWidget.cpp
    KisTagToolButton.cpp
    KisResourceTaggingManager.cpp
    KisStorageChooserWidget.cpp
    TagActions.cpp
    KisResourceOverwriteDialog.cpp
)

add_library(kritaresourcewidgets SHARED ${kritaresourcewidgets_LIB_SRCS})

generate_export_header(kritaresourcewidgets BASE_NAME kritaresourcewidgets)

target_link_libraries(kritaresourcewidgets
    PUBLIC
        Qt5::Core
        Qt5::Widgets
    PRIVATE
        Qt5::Sql
        kritaversion
        kritaglobal
        kritaplugin
        kritastore
        kritaresources
        kritawidgetutils
        KF5::ConfigCore
        KF5::WidgetsAddons
        KF5::CoreAddons
        KF5::I18n
)

set_target_properties(kritaresourcewidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresourcewidgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

