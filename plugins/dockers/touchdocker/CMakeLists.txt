set(KRITA_TOUCHDOCKER_SOURCES 
    TouchDockerPlugin.cpp 
    TouchDockerDock.cpp 
)

if(WIN32)
set(KRITA_TOUCHDOCKER_SOURCES ${KRITA_TOUCHDOCKER_SOURCES} 
    TouchDockerQQuickWidget.cpp
)
endif(WIN32)

qt6_add_resources(KRITA_TOUCHDOCKER_SOURCES touchdocker.qrc)

kis_add_library(kritatouchdocker MODULE ${KRITA_TOUCHDOCKER_SOURCES})
target_link_libraries(kritatouchdocker 
    kritaui 
    kritaqml
    Qt6::Quick  
    Qt6::QuickWidgets 
    Qt6::Qml
)
install(TARGETS kritatouchdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
