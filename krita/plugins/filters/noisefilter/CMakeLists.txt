

set(kritanoisefilter_PART_SRCS
    noisefilter.cpp
    kis_wdg_noise.cpp
    )


ki18n_wrap_ui(kritanoisefilter_PART_SRCS
    wdgnoiseoptions.ui
    )

add_library(kritanoisefilter MODULE ${kritanoisefilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritanoisefilter kritanoisefilter.desktop)


target_link_libraries(kritanoisefilter kritaui)

install(TARGETS kritanoisefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
