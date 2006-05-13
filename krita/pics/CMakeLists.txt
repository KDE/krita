

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  
    deletelayer.png  	
    lowerlayer.png  	
    newlayer.png  	
    raiselayer.png          
    visible.png  	
    novisible.png  	
    linked.png  	
    unlinked.png          
    tool_screenshot.png  	
    tablet.png  	
    locked.png  	
    unlocked.png
    )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )



#original Makefile.am contents follow:

#
#kritapics_DATA = deletelayer.png \
#	lowerlayer.png \
#	newlayer.png \
#	raiselayer.png \
#        visible.png \
#	novisible.png \
#	linked.png \
#	unlinked.png \
#        tool_screenshot.png \
#	tablet.png \
#	locked.png \
#	unlocked.png
#
## directory for pixmaps
#kritapicsdir = $(kde_datadir)/krita/pics
#
#KDE_ICON = krita
#
#
#
#
