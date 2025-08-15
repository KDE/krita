set(kritaqmlwidgets_SRCS
    KisQQuickWidget.cpp
)

kis_add_library(kritaqmlwidgets SHARED ${kritaqmlwidgets_SRCS})
generate_export_header(kritaqmlwidgets)

target_link_libraries(kritaqmlwidgets
    PRIVATE
        kritaui
        Qt::Qml
        Qt::Quick
        Qt::QuickWidgets
        Qt::QuickControls2
)

set_target_properties(kritaqmlwidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION}
    SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaqmlwidgets ${INSTALL_TARGETS_DEFAULT_ARGS})
