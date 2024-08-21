set(kritaresourcewidgets_LIB_SRCS
    KisIconToolTip.cpp
    KisResourceItemChooserContextMenu.cpp
    KisResourceItemChooser.cpp
    KisResourceItemChooserSync.cpp
    KisResourceItemDelegate.cpp
    KisResourceItemListView.cpp
    KisResourceItemListWidget.cpp
    KisResourceItemView.cpp
    KisResourceThumbnailPainter.cpp
    KisTagChooserWidget.cpp
    KisTagLabel.cpp
    KisTagFilterWidget.cpp
    KisTagToolButton.cpp
    KisResourceTaggingManager.cpp
    KisStorageChooserWidget.cpp
    TagActions.cpp
    KisResourceUserOperations.cpp
)

kis_add_library(kritaresourcewidgets SHARED ${kritaresourcewidgets_LIB_SRCS})

generate_export_header(kritaresourcewidgets BASE_NAME kritaresourcewidgets)

target_link_libraries(kritaresourcewidgets
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Widgets
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Sql
        kritaversion
        kritaglobal
        kritaplugin
        kritastore
        kritaresources
        kritawidgetutils
        KF${KF_MAJOR}::ConfigCore
        KF${KF_MAJOR}::WidgetsAddons
        KF${KF_MAJOR}::CoreAddons
        KF${KF_MAJOR}::I18n
)

set_target_properties(kritaresourcewidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresourcewidgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

