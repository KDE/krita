#include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(kritatoollazybrush_SOURCES
    tool_lazybrush.cpp
    kis_tool_lazy_brush.cpp
    )

#ki18n_wrap_ui(kritatoollazybrush_SOURCES wdgcolorpicker.ui wdgmovetool.ui)
qt5_add_resources(kritatoollazybrush_SOURCES tool_lazybrush.qrc )

add_library(kritatoollazybrush MODULE ${kritatoollazybrush_SOURCES})

generate_export_header(kritatoollazybrush BASE_NAME kritatoollazybrush)

target_link_libraries(kritatoollazybrush kritaui)
#target_link_libraries(kritatoollazybrush ${Boost_SYSTEM_LIBRARY})

install(TARGETS kritatoollazybrush  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

# install( FILES
#     KisToolPencil.action
# DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
