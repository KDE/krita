add_subdirectory(dbexplorer)

set(kritaresourcewidgets_LIB_SRCS
    KisResourceItemDelegate.cpp
    KisResourceItemView.cpp
    KisIconToolTip.cpp
    KisGridView.cpp
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
        KF5::CoreAddons
        KF5::I18n
        KF5::Archive
)

set_target_properties(kritaresourcewidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresourcewidgets ${INSTALL_TARGETS_DEFAULT_ARGS} )

