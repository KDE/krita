project( basicflakesplugin )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${BASICFLAKES_INCLUDES})

#add_subdirectory(pics)


########### Basicflakes plugin library ###############

SET ( basicflakesplugin_SRCS
    Plugin.cpp
    KoCreatePathToolFactory.cpp
    KoPencilToolFactory.cpp
)

kde4_add_ui_files(basicflakesplugin_SRCS
)

kde4_add_plugin(calligra_tool_basicflakes ${basicflakesplugin_SRCS})
target_link_libraries(calligra_tool_basicflakes flake kowidgets basicflakes)
install(TARGETS calligra_tool_basicflakes DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_tool_basicflakes.desktop DESTINATION ${SERVICES_INSTALL_DIR})

#kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
