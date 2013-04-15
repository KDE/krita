set(kritalayergroupswitcher_PART_SRCS
    layergroupswitcher.cpp
)

kde4_add_plugin(kritalayergroupswitcher ${kritalayergroupswitcher_PART_SRCS})

target_link_libraries(kritalayergroupswitcher kritaui)

install(TARGETS kritalayergroupswitcher  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritalayergroupswitcher.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
