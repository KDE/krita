project( basicflakesplugin )

include_directories( ${FLAKE_INCLUDES}  ${BASICFLAKES_INCLUDES})

#add_subdirectory(pics)


########### Basicflakes plugin library ###############

set ( basicflakesplugin_SRCS
    Plugin.cpp
    KoCreatePathToolFactory.cpp
    KoPencilToolFactory.cpp
)

ki18n_wrap_ui(basicflakesplugin_SRCS
)

add_library(calligra_tool_basicflakes MODULE ${basicflakesplugin_SRCS})
target_link_libraries(calligra_tool_basicflakes flake basicflakes)
install(TARGETS calligra_tool_basicflakes DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_tool_basicflakes.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

#ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
