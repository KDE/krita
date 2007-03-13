########### next target ###############

set(kritacpaintbrush_PART_SRCS bristle.cpp brush.cpp gauss.cpp sample.cpp stroke.cpp cpaintbrush.cc kis_cpaintop.cc )

kde4_add_ui_files(kritacpaintbrush_PART_SRCS  wdgcpaintoptions.ui)

kde4_automoc(${kritacpaintbrush_PART_SRCS})

kde4_add_plugin(kritacpaintbrush ${kritacpaintbrush_PART_SRCS})



target_link_libraries(kritacpaintbrush  ${KDE4_KDECORE_LIBS} kritaui kopainter kofficecore )

install(TARGETS kritacpaintbrush DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kritacpaintbrush.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    brush1.png
    brush2.png
    brush3.png
    brush4.png
    brush5.png	
    brush6.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
