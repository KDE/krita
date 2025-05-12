if (QT_MAJOR_VERSION STREQUAL "6")

    ecm_add_qml_module(KritaQmlPlugin
        URI org.krita.qml
        VERSION 1.0
        SHARED
        GENERATE_PLUGIN_SOURCE
    )

target_sources(KritaQmlPlugin
    PRIVATE
        SvgTextLabel.cpp
        SvgTextLabel.h
)

else ()
    ecm_add_qml_module(KritaQmlPlugin
        URI "org.krita.qml"
        VERSION 1.0
        CLASSNAME KritaQmlPlugin
    )

target_sources(KritaQmlPlugin
    PRIVATE
        KritaQmlPlugin.cpp
        KritaQmlPlugin.h
        SvgTextLabel.cpp
        SvgTextLabel.h
)

endif ()

ecm_target_qml_sources(KritaQmlPlugin SOURCES
    qml/DoubleSpinBox.qml
)

target_link_libraries(KritaQmlPlugin
    PRIVATE
        kritaui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
)

ecm_finalize_qml_module(KritaQmlPlugin DESTINATION ${KDE_INSTALL_QMLDIR})


if (QT_MAJOR_VERSION STREQUAL "5")
    ecm_generate_qmltypes(org.krita.qml 1.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/krita/qml)
endif ()


install(TARGETS KritaQmlPlugin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})



