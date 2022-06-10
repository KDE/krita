set(KRITA_THROTTLEDOCKER_SOURCES 
    ThrottlePlugin.cpp 
    Throttle.cpp 
)

qt5_add_resources(KRITA_THROTTLEDOCKER_SOURCES throttle.qrc)

kis_add_library(kritathrottledocker MODULE ${KRITA_THROTTLEDOCKER_SOURCES})
target_link_libraries(kritathrottledocker
    kritaui 
    kritaqml
    Qt5::Quick  
    Qt5::QuickWidgets 
    Qt5::Qml
)
install(TARGETS kritathrottledocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

