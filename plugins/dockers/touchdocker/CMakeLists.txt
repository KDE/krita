set(KRITA_TOUCHDOCKER_SOURCES 
    TouchDockerPlugin.cpp 
    TouchDockerDock.cpp 
)

if(WIN32)
set(KRITA_TOUCHDOCKER_SOURCES ${KRITA_TOUCHDOCKER_SOURCES} 
    TouchDockerQQuickWidget.cpp
)
endif(WIN32)

qt5_add_resources(KRITA_TOUCHDOCKER_SOURCES touchdocker.qrc)

add_library(kritatouchdocker MODULE ${KRITA_TOUCHDOCKER_SOURCES})
target_link_libraries(kritatouchdocker 
    kritaui 
    kritaqml
    Qt5::Quick  
    Qt5::QuickWidgets 
    Qt5::Qml
)
install(TARGETS kritatouchdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
