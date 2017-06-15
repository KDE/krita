set(kritatoolreferenceimages_SOURCES
    Plugin.cpp
    ToolReferenceImages.cpp
    #ToolReferenceImagesWidget.cpp
    )

#ki18n_wrap_ui(kritatoolreferenceimages_SOURCES WdgToolReferenceImagesOptions.ui)

add_library(kritatoolreferenceimages MODULE ${kritatoolreferenceimages_SOURCES})

target_link_libraries(kritatoolreferenceimages kritaui)

install(TARGETS kritatoolreferenceimages  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

