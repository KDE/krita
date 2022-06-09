project(calligradockers)

set(calligradockers_SRCS
    SvgSymbolCollectionDocker.cpp
    Plugin.cpp
)

ki18n_wrap_ui(calligradockers_SRCS
    WdgSvgCollection.ui
)


kis_add_library(kritasvgcollectiondocker MODULE ${calligradockers_SRCS} PCH_FILE kis_global_pch.h)

target_link_libraries(kritasvgcollectiondocker kritawidgets kritaui)

install(TARGETS kritasvgcollectiondocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
