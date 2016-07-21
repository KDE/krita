
set(kritaremotecontrol_SOURCES
    remotecontrol.cpp
    kis_opcua_server.cpp
    kis_paintop_control_object.cpp
    open62541.c
    )

add_library(kritaremotecontrol MODULE ${kritaremotecontrol_SOURCES})
set_property(TARGET kritaremotecontrol PROPERTY C_STANDARD 99)

target_link_libraries(kritaremotecontrol kritaui)
install(TARGETS kritaremotecontrol  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})