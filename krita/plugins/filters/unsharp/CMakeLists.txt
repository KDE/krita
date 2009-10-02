

set(kritaunsharpfilter_PART_SRCS
    unsharp.cpp
    kis_wdg_unsharp.cpp
    kis_unsharp_filter.cpp
    )

kde4_add_ui_files(kritaunsharpfilter_PART_SRCS
    wdgunsharp.ui
    )

kde4_add_plugin(kritaunsharpfilter ${kritaunsharpfilter_PART_SRCS})



target_link_libraries(kritaunsharpfilter kritaui)

install(TARGETS kritaunsharpfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaunsharpfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
