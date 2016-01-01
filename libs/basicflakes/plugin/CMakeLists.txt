project( basicflakesplugin )

include_directories( ${FLAKE_INCLUDES}  ${BASICFLAKES_INCLUDES})

#add_subdirectory(pics)


########### Basickritaflakes plugin library ###############

set ( basicflakesplugin_SRCS
    Plugin.cpp
    KoCreatePathToolFactory.cpp
    KoPencilToolFactory.cpp
)

ki18n_wrap_ui(basicflakesplugin_SRCS
)

add_library(krita_tool_basicflakes MODULE ${basicflakesplugin_SRCS})
target_link_libraries(krita_tool_basicflakes kritaflake kritabasicflakes)
install(TARGETS krita_tool_basicflakes DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
