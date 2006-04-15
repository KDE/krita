kde4_header()




########### install files ###############

kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons crystalsvg)
install_files( ${DATA_INSTALL_DIR}/krita/templates/cmyk/.source FILES  white_2000x800.kra )
install_files( ${DATA_INSTALL_DIR}/krita/templates/cmyk FILES  .directory white_2000x800.desktop )

kde4_footer()



#original Makefile.am contents follow:

#templates_DATA = .directory white_2000x800.desktop
#templatesdir = $(kde_datadir)/krita/templates/cmyk
#
#templatesrc_DATA = white_2000x800.kra
#templatesrcdir = $(kde_datadir)/krita/templates/cmyk/.source
#
#templatesicon_ICON = AUTO
#templatesicondir = $(kde_datadir)/krita/icons
