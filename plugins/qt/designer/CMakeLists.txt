find_package(Qt5 ${MIN_QT_VERSION}
    REQUIRED COMPONENTS
    Designer
)

include_directories(${Qt5Designer_INCLUDE_DIRS})

set(KritaDesignerPlugin_SOURCES
    KisColorSpaceSelectorPlugin.cpp
    KritaDesignerPluginCollection.cpp
)

add_library(kritadesignerplugin MODULE ${KritaDesignerPlugin_SOURCES})

target_link_libraries(kritadesignerplugin kritaui)
