

set(kritaunsharpfilter_PART_SRCS
    unsharp.cc
    kis_wdg_unsharp.cc
    kis_unsharp_filter.cc
    )

kde4_automoc(${kritaunsharpfilter_PART_SRCS})

kde4_add_ui_files(kritaunsharpfilter_PART_SRCS
    wdgunsharp.ui
    )

kde4_add_plugin(kritaunsharpfilter ${kritaunsharpfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaunsharpfilter )

target_link_libraries(kritaunsharpfilter kritaui)

install(TARGETS kritaunsharpfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaunsharpfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
