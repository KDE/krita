set(KRITA_TEXTPROPERTIES_SOURCES 
    TextPropertiesPlugin.cpp 
    TextPropertiesDock.cpp 
)

qt5_add_resources(KRITA_TEXTPROPERTIES_SOURCES textproperties.qrc)

kis_add_library(kritatextproperties MODULE ${KRITA_TEXTPROPERTIES_SOURCES})
target_link_libraries(kritatextproperties 
    kritaui
    Qt5::Quick  
    Qt5::QuickWidgets 
    Qt5::Qml
)
install(TARGETS kritatextproperties  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
 
