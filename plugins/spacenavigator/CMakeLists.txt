project( spacenavigator )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOMAIN_INCLUDES}  ${SPNAV_INCLUDE_DIR})


########### Flake Plugin library ###############

SET ( spacenavigator_SRCS
    Plugin.cpp
    SpaceNavigatorDevice.cpp
    SpaceNavigatorPollingThread.cpp
    SpaceNavigatorEvent.cpp
)

kde4_add_plugin(calligra_device_spacenavigator ${spacenavigator_SRCS})
target_link_libraries(calligra_device_spacenavigator ${SPNAV_LIBRARY} ${X11_LIBRARIES} kotext)
install(TARGETS calligra_device_spacenavigator DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES spacenavigator.desktop DESTINATION ${SERVICES_INSTALL_DIR})
