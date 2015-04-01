

if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
endif()


########### next target ###############

set(krita_raw_import_PART_SRCS kis_raw_import.cpp )


ki18n_wrap_ui(krita_raw_import_PART_SRCS wdgrawimport.ui )

add_library(krita_raw_import MODULE ${krita_raw_import_PART_SRCS})

target_link_libraries(krita_raw_import kritaui  KF5::KDcraw)

install(TARGETS krita_raw_import  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_raw_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_raw.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

