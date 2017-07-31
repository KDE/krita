set(KRITA_TOUCHDOCKER_SOURCES 
    ThrottlePlugin.cpp 
    Throttle.cpp 
)

qt5_add_resources(KRITA_TOUCHDOCKER_SOURCES throttle.qrc)

add_library(kritathrottle MODULE ${KRITA_TOUCHDOCKER_SOURCES})
target_link_libraries(kritathrottle
    kritaui 
    kritaqml
    Qt5::Quick  
    Qt5::QuickWidgets 
    Qt5::Qml
)
install(TARGETS kritathrottle DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
