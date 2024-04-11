find_package(Qt6 ${MIN_QT_VERSION}
    REQUIRED COMPONENTS
    Designer
)

include_directories(${Qt6Designer_INCLUDE_DIRS})

set(KritaDesignerPlugin_SOURCES
    KisColorSpaceSelectorPlugin.cpp
    KritaDesignerPluginCollection.cpp
)

kis_add_library(kritadesignerplugin MODULE ${KritaDesignerPlugin_SOURCES})

target_link_libraries(kritadesignerplugin kritaui)
