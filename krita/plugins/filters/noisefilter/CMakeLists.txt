

set(kritanoisefilter_PART_SRCS
    noisefilter.cpp
    kis_wdg_noise.cpp
    )


ki18n_wrap_ui(kritanoisefilter_PART_SRCS
    wdgnoiseoptions.ui
    )

add_library(kritanoisefilter MODULE ${kritanoisefilter_PART_SRCS})



target_link_libraries(kritanoisefilter kritaui)

install(TARGETS kritanoisefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanoisefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
