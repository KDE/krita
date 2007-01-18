include_directories( ${KOFFICEUI_INCLUDES} )
add_subdirectory(pics)


########### Flake Plugin library ###############

SET (kritashape_LIB_SRCS
	KritaShape.cpp
	KritaShapeFactory.cpp
	KritaShapeTool.cpp
	KritaShapeToolFactory.cpp
	)

kde4_automoc(${kritashape_LIB_SRCS})
kde4_add_plugin(kritashape ${kritashape_LIB_SRCS})

target_link_libraries(kritashape flake kofficeui kritaui)
install(TARGETS kritashape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kritashape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
