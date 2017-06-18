project(calligradockers)

set(calligradockers_SRCS
    ShapeCollectionDocker.cpp
    CollectionItemModel.cpp
    SvgSymbolCollectionDocker.cpp

    Plugin.cpp
)

ki18n_wrap_ui(calligradockers_SRCS
    WdgSvgCollection.ui
)


add_library(krita_docker_defaults MODULE ${calligradockers_SRCS})

target_link_libraries(krita_docker_defaults kritawidgets kritaui)

install(TARGETS krita_docker_defaults DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
