kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kformula/fonts FILES  cmbx10.ttf cmex10.ttf cmmi10.ttf cmr10.ttf cmsy10.ttf msam10.ttf msbm10.ttf )

kde4_footer()



#original Makefile.am contents follow:

#fontdir = $(prefix)/share/apps/kformula/fonts
#font_DATA = cmbx10.ttf cmex10.ttf cmmi10.ttf cmr10.ttf cmsy10.ttf msam10.ttf msbm10.ttf
#
#EXTRA_DIST = $(font_DATA) LICENCE
#
## This uses -hook instead of -local so that it is run after fonts are installed
#install-data-hook:
#		-mkfontscale $(DESTDIR)$(fontdir)
#		-mkfontdir $(DESTDIR)$(fontdir)
#
#
