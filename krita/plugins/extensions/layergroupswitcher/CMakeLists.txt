set(kritalayergroupswitcher_PART_SRCS
    layergroupswitcher.cpp
)

add_library(kritalayergroupswitcher MODULE ${kritalayergroupswitcher_PART_SRCS})
kcoreaddons_desktop_to_json(kritalayergroupswitcher kritalayergroupswitcher.desktop)

target_link_libraries(kritalayergroupswitcher kritaui)

install(TARGETS kritalayergroupswitcher  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  kritalayergroupswitcher.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
