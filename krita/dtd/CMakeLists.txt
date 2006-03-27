kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/dtd FILES  krita.dtd )

kde4_footer()



#original Makefile.am contents follow:

#dtd_DATA = krita.dtd
#
#dtddir = $(kde_datadir)/krita/dtd
#
