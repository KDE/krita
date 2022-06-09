set(kritalayergroupswitcher_SOURCES
    layergroupswitcher.cpp
)

kis_add_library(kritalayergroupswitcher MODULE ${kritalayergroupswitcher_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritalayergroupswitcher kritaui)
install(TARGETS kritalayergroupswitcher  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
