

set(kritaposterize_PART_SRCS posterize.cpp )

add_library(kritaposterize MODULE ${kritaposterize_PART_SRCS})



target_link_libraries(kritaposterize kritaui)

install(TARGETS kritaposterize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaposterize.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


