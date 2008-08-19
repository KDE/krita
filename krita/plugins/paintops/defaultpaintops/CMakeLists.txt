

set(kritadefaultpaintops_PART_SRCS
	defaultpaintops_plugin.cc
	kis_airbrushop.cc
	kis_brushop.cc
	kis_duplicateop.cc
	kis_eraseop.cc
	kis_penop.cc
#    kis_smudgeop.cc   
    )

kde4_add_ui_files(kritadefaultpaintops_PART_SRCS wdgbrushcurves.ui wdgduplicateop.ui )

kde4_add_plugin(kritadefaultpaintops ${kritadefaultpaintops_PART_SRCS})



target_link_libraries(kritadefaultpaintops kritaui kritalibpaintop)

install(TARGETS kritadefaultpaintops  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadefaultpaintops.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-airbrush.png
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
    krita-pencil.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


