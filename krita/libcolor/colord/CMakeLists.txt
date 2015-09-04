set(kritacolord_SRCS
    KisColord.cpp
)

set(COLORD_INTERFACE_XML
    org.freedesktop.ColorManager.xml
)
set(COLORD_DEVICE_INTERFACE_XML
    org.freedesktop.ColorManager.Device.xml
)
set(COLORD_PROFILE_INTERFACE_XML
    org.freedesktop.ColorManager.Profile.xml
)
set(COLORD_SENSOR_INTERFACE_XML
    org.freedesktop.ColorManager.Sensor.xml
)

set_source_files_properties(${COLORD_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_INTERFACE_XML} PROPERTIES CLASSNAME CdInterface)
qt4_add_dbus_interface(kritacolord_SRCS ${COLORD_INTERFACE_XML} CdInterface)

set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES CLASSNAME CdDeviceInterface)
qt4_add_dbus_interface(kritacolord_SRCS ${COLORD_DEVICE_INTERFACE_XML} CdDeviceInterface)

set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES CLASSNAME CdProfileInterface)
qt4_add_dbus_interface(kritacolord_SRCS ${COLORD_PROFILE_INTERFACE_XML} CdProfileInterface)

set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES CLASSNAME CdSensorInterface)
qt4_add_dbus_interface(kritacolord_SRCS ${COLORD_SENSOR_INTERFACE_XML} CdSensorInterface)

kde4_add_library(kritacolord SHARED ${kritacolord_SRCS})
target_link_libraries(kritacolord kritaglobal ${KDE4_KDEUI_LIBS})

install(TARGETS kritacolord DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
