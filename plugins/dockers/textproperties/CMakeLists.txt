set(KRITA_TEXTPROPERTIES_SOURCES 
    TextPropertiesPlugin.cpp 
    TextPropertiesDock.cpp 
    FontStyleModel.cpp
    FontAxesModel.cpp
    KoShapeQtQuickLabel.cpp
    TagFilterProxyModelQmlWrapper.cpp
    OpenTypeFeatureModel.cpp
    LocaleHandler.cpp
    CssQmlUnitConverter.cpp
    TextPropertyConfigModel.cpp
    TextPropertyConfigDialog.cpp
    CssStylePresetEditDialog.cpp
)

qt_add_resources(KRITA_TEXTPROPERTIES_SOURCES textproperties.qrc)

kis_add_library(kritatextproperties MODULE ${KRITA_TEXTPROPERTIES_SOURCES})
target_link_libraries(kritatextproperties 
    kritaui
    kritaqmlwidgets
    Qt${QT_MAJOR_VERSION}::Quick  
    Qt${QT_MAJOR_VERSION}::QuickWidgets 
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Qml
)
install(TARGETS kritatextproperties  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
 
