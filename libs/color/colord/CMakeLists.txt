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
qt5_add_dbus_interface(kritacolord_SRCS ${COLORD_INTERFACE_XML} CdInterface)

set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_DEVICE_INTERFACE_XML} PROPERTIES CLASSNAME CdDeviceInterface)
qt5_add_dbus_interface(kritacolord_SRCS ${COLORD_DEVICE_INTERFACE_XML} CdDeviceInterface)

set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_PROFILE_INTERFACE_XML} PROPERTIES CLASSNAME CdProfileInterface)
qt5_add_dbus_interface(kritacolord_SRCS ${COLORD_PROFILE_INTERFACE_XML} CdProfileInterface)

set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES INCLUDE "dbus-types.h")
set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${COLORD_SENSOR_INTERFACE_XML} PROPERTIES CLASSNAME CdSensorInterface)
qt5_add_dbus_interface(kritacolord_SRCS ${COLORD_SENSOR_INTERFACE_XML} CdSensorInterface)


kis_add_core_library(kritacolord SHARED ${kritacolord_SRCS})
target_link_libraries(kritacolord kritaglobal Qt5::DBus Qt5::Concurrent Qt5::Core Qt5::Gui)
generate_export_header(kritacolord BASE_NAME kritacolord)

install(TARGETS kritacolord DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
