kde4_header()




########### install files ###############

kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons crystalsvg)
install_files( ${DATA_INSTALL_DIR}/krita/templates/gray/.source FILES  white_640x480.kra  )
install_files( ${DATA_INSTALL_DIR}/krita/templates/gray FILES  .directory white_640x480.desktop )

kde4_footer()



#original Makefile.am contents follow:

#templates_DATA = .directory white_640x480.desktop
#templatesdir = $(kde_datadir)/krita/templates/gray
#
#templatesrc_DATA =white_640x480.kra 
#templatesrcdir = $(kde_datadir)/krita/templates/gray/.source
#
#templatesicon_ICON = AUTO
#templatesicondir = $(kde_datadir)/krita/icons
