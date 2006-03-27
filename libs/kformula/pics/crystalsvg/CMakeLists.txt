kde4_header()




########### install files ###############

install_files( ${KDE4_ICON_DIR}/hicolor FILES  )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#KDE_ICON=AUTO
#
#hicolordir=$(kde_icondir)/hicolor
#
#
