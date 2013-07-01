include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

SET (templateshape_LIB_SRCS
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

kde4_add_plugin(calligra_shape_template ${templateshape_LIB_SRCS})

target_link_libraries(calligra_shape_template flake    ${KDE4_KFILE_LIBRARY})
#Template: Possibly you need to link with komain too.
#target_link_libraries(templateshape komain flake    ${KDE4_KFILE_LIBRARY})

install(TARGETS calligra_shape_template DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES templateshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
