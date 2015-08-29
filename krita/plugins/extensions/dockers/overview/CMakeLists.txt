set(KRITA_OVERVIEWDOCKER_PART_SRCS overviewdocker.cpp overviewdocker_dock.cpp overviewwidget.cc )

add_library(kritaoverviewdocker MODULE ${KRITA_OVERVIEWDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritaoverviewdocker krita_overviewdocker.desktop)

target_link_libraries(kritaoverviewdocker kritaui)

install(TARGETS kritaoverviewdocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_overviewdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

