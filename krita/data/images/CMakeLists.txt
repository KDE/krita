kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files(  FILES  previewfilter.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kritaimagesdir = $(prefix)/share/apps/krita/images
#
## kritaimages_DATA = evenings.jpg hakonepa.jpg hiro_awate.jpg Azay-Le-Rideau.jpg previewfilter.png
#kritaimages_DATA = previewfilter.png
#
