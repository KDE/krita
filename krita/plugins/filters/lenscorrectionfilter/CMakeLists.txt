

set(kritalenscorrectionfilter_PART_SRCS
    lenscorrectionfilter.cc
    kis_wdg_lens_correction.cpp
    )

kde4_automoc(${kritalenscorrectionfilter_PART_SRCS})

kde4_add_ui_files(kritalenscorrectionfilter_PART_SRCS
    wdglenscorrectionoptions.ui
    )

kde4_add_plugin(kritalenscorrectionfilter ${kritalenscorrectionfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritalenscorrectionfilter )

target_link_libraries(kritalenscorrectionfilter kritaui)

install(TARGETS kritalenscorrectionfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritalenscorrectionfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
