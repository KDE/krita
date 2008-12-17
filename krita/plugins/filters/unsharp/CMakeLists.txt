

set(kritaunsharpfilter_PART_SRCS
    unsharp.cc
    kis_wdg_unsharp.cc
    kis_unsharp_filter.cc
    )

kde4_add_ui_files(kritaunsharpfilter_PART_SRCS
    wdgunsharp.ui
    )

kde4_add_plugin(kritaunsharpfilter ${kritaunsharpfilter_PART_SRCS})



target_link_libraries(kritaunsharpfilter koguiutils kritaimage kritaui ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritaunsharpfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaunsharpfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
