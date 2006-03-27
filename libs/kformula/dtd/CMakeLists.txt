kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kformula/dtd FILES  kformula.dtd )

kde4_footer()



#original Makefile.am contents follow:

#dtd_DATA = kformula.dtd
#
#dtddir = $(kde_datadir)/kformula/dtd
#
