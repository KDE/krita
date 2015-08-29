project( spacenavigator )

include_directories( ${FLAKE_INCLUDES}  ${SPNAV_INCLUDE_DIR})


########### Flake Plugin library ###############

set ( spacenavigator_SRCS
    Plugin.cpp
    SpaceNavigatorDevice.cpp
    SpaceNavigatorPollingThread.cpp
    SpaceNavigatorEvent.cpp
)

add_library(calligra_device_spacenavigator MODULE ${spacenavigator_SRCS})
kcoreaddons_desktop_to_json(calligra_device_spacenavigator calligra_device_spacenavigator.desktop)
target_link_libraries(calligra_device_spacenavigator ${SPNAV_LIBRARY} ${X11_LIBRARIES} kotext)
install(TARGETS calligra_device_spacenavigator DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
