kde4_header()

add_subdirectory( crystalsvg ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = crystalsvg
#
