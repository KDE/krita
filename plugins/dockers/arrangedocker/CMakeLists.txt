set(KRITA_ARRANGEDOCKER_SOURCES arrangedocker.cpp arrangedocker_dock.cpp arrange_docker_widget.cpp)
ki18n_wrap_ui(KRITA_ARRANGEDOCKER_SOURCES
    arrange_docker_widget.ui
)

kis_add_library(kritaarrangedocker MODULE ${KRITA_ARRANGEDOCKER_SOURCES} PCH_FILE kis_global_pch.h)
target_link_libraries(kritaarrangedocker kritaui)
install(TARGETS kritaarrangedocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
