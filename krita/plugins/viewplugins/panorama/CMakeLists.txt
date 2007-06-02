
set(kritapanorama_PART_SRCS panorama.cc harris_detector.cpp matching.cpp kis_interest_points_detector.cpp kis_images_blender.cpp)

kde4_automoc(${kritapanorama_PART_SRCS})

kde4_add_ui_files(kritapanorama_PART_SRCS wdgpanoramacreation.ui )

kde4_add_plugin(kritapanorama ${kritapanorama_PART_SRCS})

target_link_libraries(kritapanorama kritaui)

install(TARGETS kritapanorama  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  panorama.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritapanorama.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

