set(KRITA_LOGDOCKER_SOURCES 
    LogDocker.cpp 
    LogDockerDock.cpp
)

ki18n_wrap_ui(KRITA_LOGDOCKER_SOURCES 
    WdgLogDocker.ui
)

kis_add_library(kritalogdocker MODULE ${KRITA_LOGDOCKER_SOURCES})
target_link_libraries(kritalogdocker kritaui)
if (QT_MAJOR_VERSION STREQUAL "6")
target_link_libraries(kritalogdocker KF6::ColorScheme)
endif()
install(TARGETS kritalogdocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
