set(KRITA_DIGITALMIXER_SOURCES digitalmixer.cc digitalmixer_dock.cc )
kis_add_library(kritadigitalmixer MODULE ${KRITA_DIGITALMIXER_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritadigitalmixer kritaui)
install(TARGETS kritadigitalmixer  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
