kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/koffice/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#
#kofficedir = $(kde_datadir)/koffice/icons
#koffice_ICON = AUTO
#
