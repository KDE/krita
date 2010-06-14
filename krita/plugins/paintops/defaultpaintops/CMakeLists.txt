include_directories(brush)
include_directories(pen)
include_directories(eraser)
include_directories(duplicate)
include_directories(smudge)


set(kritadefaultpaintops_PART_SRCS
	defaultpaintops_plugin.cc
	brush/kis_brushop.cpp
	brush/kis_brushop_settings_widget.cpp
	duplicate/kis_duplicateop.cpp
	duplicate/kis_duplicateop_factory.cpp
	duplicate/kis_duplicateop_settings.cpp
	duplicate/kis_duplicateop_settings_widget.cpp
	duplicate/kis_duplicateop_option.cpp
	pen/kis_penop.cpp
	pen/kis_penop_settings_widget.cpp
	smudge/kis_smudgeop.cpp
	smudge/kis_smudgeop_settings_widget.cpp
    )

kde4_add_ui_files(kritadefaultpaintops_PART_SRCS duplicate/wdgduplicateop.ui )

kde4_add_plugin(kritadefaultpaintops ${kritadefaultpaintops_PART_SRCS})



target_link_libraries(kritadefaultpaintops kritalibpaintop)

install(TARGETS kritadefaultpaintops  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadefaultpaintops.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-smudgebrush.png
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
    krita-pencil.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


install( FILES  
    smudge/smudge-finger.obj 
    smudge/smudge-finger.mtl DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )

