




########### install files ###############

kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons crystalsvg)
install( FILES  white_2000x800.kra  DESTINATION ${DATA_INSTALL_DIR}/krita/templates/cmyk/.source)
install( FILES  .directory white_2000x800.desktop  DESTINATION ${DATA_INSTALL_DIR}/krita/templates/cmyk)





#original Makefile.am contents follow:

#templates_DATA = .directory white_2000x800.desktop
#templatesdir = $(kde_datadir)/krita/templates/cmyk
#
#templatesrc_DATA = white_2000x800.kra
#templatesrcdir = $(kde_datadir)/krita/templates/cmyk/.source
#
#templatesicon_ICON = AUTO
#templatesicondir = $(kde_datadir)/krita/icons
