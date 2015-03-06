set(KRITA_OVERVIEWDOCKER_PART_SRCS overviewdocker.cpp overviewdocker_dock.cpp overviewwidget.cc )

add_library(kritaoverviewdocker MODULE ${KRITA_OVERVIEWDOCKER_PART_SRCS})

target_link_libraries(kritaoverviewdocker kritaui)

install(TARGETS kritaoverviewdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_overviewdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

