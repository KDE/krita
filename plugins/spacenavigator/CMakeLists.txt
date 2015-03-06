project( spacenavigator )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${SPNAV_INCLUDE_DIR})


########### Flake Plugin library ###############

set ( spacenavigator_SRCS
    Plugin.cpp
    SpaceNavigatorDevice.cpp
    SpaceNavigatorPollingThread.cpp
    SpaceNavigatorEvent.cpp
)

add_library(calligra_device_spacenavigator MODULE ${spacenavigator_SRCS})
target_link_libraries(calligra_device_spacenavigator ${SPNAV_LIBRARY} ${X11_LIBRARIES} kotext)
install(TARGETS calligra_device_spacenavigator DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_device_spacenavigator.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
