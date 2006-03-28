kde4_header()

add_subdirectory( brushes )
add_subdirectory( patterns )
add_subdirectory( gradients )
add_subdirectory( profiles )
add_subdirectory( palettes )
add_subdirectory( images )




########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES  krita_filter.desktop      krita_paintop.desktop      krita_plugin.desktop      krita_tool.desktop )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . brushes patterns gradients profiles palettes images
#kde_servicetypes_DATA = \
#    krita_filter.desktop \
#    krita_paintop.desktop \
#    krita_plugin.desktop \
#    krita_tool.desktop
#    
