

kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons )
install( FILES  transparent_1024x768.kra transparent_1280x1024.kra transparent_1600x1200.kra transparent_640x480.kra white_1024x768.kra white_1280x1024.kra white_1600x1200.kra white_640x480.kra  DESTINATION ${DATA_INSTALL_DIR}/krita/templates/rgb/.source)
install( FILES  .directory transparent_1024x768.desktop transparent_1280x1024.desktop transparent_1600x1200.desktop transparent_640x480.desktop white_1024x768.desktop white_1280x1024.desktop white_1600x1200.desktop white_640x480.desktop  DESTINATION ${DATA_INSTALL_DIR}/krita/templates/rgb)





#original Makefile.am contents follow:

#templates_DATA = .directory transparent_1024x768.desktop transparent_1280x1024.desktop transparent_1600x1200.desktop transparent_640x480.desktop white_1024x768.desktop white_1280x1024.desktop white_1600x1200.desktop white_640x480.desktop
#templatesdir = $(kde_datadir)/krita/templates/rgb
#
#templatesrc_DATA = transparent_1024x768.kra transparent_1280x1024.kra transparent_1600x1200.kra transparent_640x480.kra white_1024x768.kra white_1280x1024.kra white_1600x1200.kra white_640x480.kra
#templatesrcdir = $(kde_datadir)/krita/templates/rgb/.source
#
#templatesicon_ICON = AUTO
#templatesicondir = $(kde_datadir)/krita/icons
