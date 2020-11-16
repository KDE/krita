set(kritaseexprgenerator_SOURCES
    SeExprVariable.cpp
    SeExprExpressionContext.cpp
    generator.cpp
    kis_wdg_seexpr.cpp
    kis_wdg_seexpr_presets_save.cpp
    )


ki18n_wrap_ui(kritaseexprgenerator_SOURCES
    wdgseexpr.ui
    wdgseexprsavepreset.ui
    )

add_library(kritaseexprgenerator MODULE ${kritaseexprgenerator_SOURCES})

target_link_libraries(kritaseexprgenerator kritaui KSeExpr::KSeExpr KSeExpr::KSeExprUI)

install(TARGETS kritaseexprgenerator  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
