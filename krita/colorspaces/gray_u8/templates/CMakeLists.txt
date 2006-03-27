kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/icons FILES  )
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
