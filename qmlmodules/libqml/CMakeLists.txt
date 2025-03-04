ecm_add_qml_module(kritaqml
    URI org.krita.qml
    VERSION 1.0
    SHARED
    GENERATE_PLUGIN_SOURCE
)

target_sources(kritaqml
    PUBLIC
        SvgTextLabel.cpp
)

target_link_libraries(kritaqml
    PUBLIC
        kritaui
    PRIVATE
        Qt::Quick
)

ecm_target_qml_sources(kritaqml SOURCES
    qml/DoubleSpinBox.qml
)

ecm_finalize_qml_module(kritaqml DESTINATION ${KDE_INSTALL_QMLDIR})

install(TARGETS kritaqml ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

