set(kritalayergroupswitcher_SOURCES
    layergroupswitcher.cpp
)

add_library(kritalayergroupswitcher MODULE ${kritalayergroupswitcher_SOURCES})
kcoreaddons_desktop_to_json(kritalayergroupswitcher kritalayergroupswitcher.desktop)

target_link_libraries(kritalayergroupswitcher kritaui)

install(TARGETS kritalayergroupswitcher  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
