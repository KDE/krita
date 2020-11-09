set(kritagradientgenerator_SOURCES
    KisGradientGeneratorPlugin.cpp
    KisGradientGenerator.cpp
    KisGradientGeneratorConfigWidget.cpp
    KisGradientGeneratorConfiguration.cpp
    )
ki18n_wrap_ui(kritagradientgenerator_SOURCES
    KisGradientGeneratorConfigWidget.ui
    )

add_library(kritagradientgenerator MODULE ${kritagradientgenerator_SOURCES})
target_link_libraries(kritagradientgenerator kritaui)
install(TARGETS kritagradientgenerator DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
