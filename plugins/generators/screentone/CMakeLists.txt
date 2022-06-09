set(kritascreentonegenerator_SOURCES
    KisScreentoneGenerator.cpp
    KisScreentoneConfigWidget.cpp
    KisScreentoneScreentoneFunctions.cpp
    KisScreentoneBrightnessContrastFunctions.cpp
    KisScreentoneGeneratorPlugin.cpp
    KisScreentoneGeneratorConfiguration.cpp
    KisScreentoneGeneratorTemplate.cpp
    )
ki18n_wrap_ui(kritascreentonegenerator_SOURCES
    KisScreentoneConfigWidget.ui
    )

kis_add_library(kritascreentonegenerator MODULE ${kritascreentonegenerator_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritascreentonegenerator kritaui)
install(TARGETS kritascreentonegenerator DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
