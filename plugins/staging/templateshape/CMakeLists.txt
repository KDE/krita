include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

set (templateshape_LIB_SRCS
    TemplateShapePlugin.cpp
    TemplateShapeFactory.cpp
    TemplateShape.cpp

    # Template: Disable these here in CMakelists.txt if you only want
    # viewing, i.e. no editing and manipulation
    TemplateToolFactory.cpp     
    TemplateTool.cpp

    # Template: Enable this if your tool needs a tool configuration
    # widget and not only use direct manipulation with mouse,
    # keyboard, tablet or similar.
    #
    # The example code in TemplateTool.cpp creates the option widget
    # directly by populating a QWidget but you could also create a
    # separate class for this.
    #
    #TemplateShapeConfigWidget.cpp

    # Template: Add more source files here if needed
    #Something.cpp

    # Template: All commands go here
    #ChangeSomethingCommand.cpp
)

add_library(calligra_shape_template MODULE ${templateshape_LIB_SRCS})
kcoreaddons_desktop_to_json(calligra_shape_template calligra_shape_template.desktop)

target_link_libraries(calligra_shape_template flake    ${KDE4_KFILE_LIBRARY})
#Template: Possibly you need to link with komain too.
#target_link_libraries(templateshape komain flake    ${KDE4_KFILE_LIBRARY})

install(TARGETS calligra_shape_template DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
