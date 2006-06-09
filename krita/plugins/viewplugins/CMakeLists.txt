

add_subdirectory( performancetest )
add_subdirectory( colorrange )
add_subdirectory( colorspaceconversion )
add_subdirectory( dropshadow )
add_subdirectory( filtersgallery )
add_subdirectory( histogram )
add_subdirectory( histogram_docker )
add_subdirectory( imagesize )
add_subdirectory( modify_selection )
add_subdirectory( rotateimage )
add_subdirectory( screenshot )
add_subdirectory( separate_channels )
add_subdirectory( shearimage )
#add_subdirectory( flakeshapes )

if(KOFFICE_ENABLE_SCRIPTING)
	add_subdirectory(scripting)
endif(KOFFICE_ENABLE_SCRIPTING)	


########### install files ###############






#original Makefile.am contents follow:

#if compile_kross
#SCRIPTINGDIR = scripting
#endif
#
#SUBDIRS = \
#    performancetest \
#    colorrange \
#    colorspaceconversion \
#    dropshadow \
#    filtersgallery \
#    histogram \
#    histogram_docker \
#    imagesize \
#    modify_selection \
#    rotateimage \
#    screenshot \
#    separate_channels \
#    shearimage \
#    $(SCRIPTINGDIR)
#
## variations
## history_docker
