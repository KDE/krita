kde4_header()

add_subdirectory( defaultpaintops ) 
add_subdirectory( smearybrush ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = \
#    defaultpaintops \
#    smearybrush
