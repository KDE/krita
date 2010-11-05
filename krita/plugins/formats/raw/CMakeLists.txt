

if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
endif(OPENEXR_FOUND)


########### next target ###############

set(krita_raw_import_PART_SRCS kis_raw_import.cpp )


kde4_add_ui_files(krita_raw_import_PART_SRCS wdgrawimport.ui )

kde4_add_plugin(krita_raw_import ${krita_raw_import_PART_SRCS})

add_definitions( ${KDCRAW_DEFINITIONS} )
include_directories( ${KDCRAW_INCLUDE_DIR} )


target_link_libraries(krita_raw_import kritaui  ${KDCRAW_LIBRARIES})

install(TARGETS krita_raw_import  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_raw_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_raw.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

