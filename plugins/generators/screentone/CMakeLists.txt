set(kritascreentonegenerator_SOURCES
    KisScreentoneGenerator.cpp
    KisScreentoneConfigWidget.cpp
    KisScreentoneScreentoneFunctions.cpp
    KisScreentoneBrightnessContrastFunctions.cpp
    KisScreentoneGeneratorPlugin.cpp
    KisScreentoneGeneratorConfiguration.cpp
    )
ki18n_wrap_ui(kritascreentonegenerator_SOURCES
    KisScreentoneConfigWidget.ui
    )

add_library(kritascreentonegenerator MODULE ${kritascreentonegenerator_SOURCES})
target_link_libraries(kritascreentonegenerator kritaui)
install(TARGETS kritascreentonegenerator DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
