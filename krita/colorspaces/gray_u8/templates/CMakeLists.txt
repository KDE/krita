




########### install files ###############

kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons )
install( FILES  white_640x480.kra   DESTINATION ${DATA_INSTALL_DIR}/krita/templates/gray/.source)
install( FILES  .directory white_640x480.desktop  DESTINATION ${DATA_INSTALL_DIR}/krita/templates/gray)





#original Makefile.am contents follow:

#templates_DATA = .directory white_640x480.desktop
#templatesdir = $(kde_datadir)/krita/templates/gray
#
#templatesrc_DATA =white_640x480.kra 
#templatesrcdir = $(kde_datadir)/krita/templates/gray/.source
#
#templatesicon_ICON = AUTO
#templatesicondir = $(kde_datadir)/krita/icons
