kde4_header()

add_subdirectory( filters ) 
add_subdirectory( paintops ) 
add_subdirectory( tools ) 
add_subdirectory( viewplugins ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = \
#    filters \
#    paintops \
#    tools \
#    viewplugins 
