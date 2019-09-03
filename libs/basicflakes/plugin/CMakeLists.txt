project( basicflakesplugin )

########### Basickritaflakes plugin library ###############

set ( basicflakesplugin_SRCS
    Plugin.cpp
    KoCreatePathToolFactory.cpp
    KoPencilToolFactory.cpp
)

ki18n_wrap_ui(basicflakesplugin_SRCS
)

kis_add_core_library(krita_tool_basicflakes MODULE ${basicflakesplugin_SRCS})
target_link_libraries(krita_tool_basicflakes kritaflake kritabasicflakes)
install(TARGETS krita_tool_basicflakes DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
