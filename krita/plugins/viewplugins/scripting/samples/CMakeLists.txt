kde4_header()

add_subdirectory( python )
add_subdirectory( ruby )

include_directories( ${CMAKE_SOURCE_DIR}/kexi/scriptingcore/main ${KDE4_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../../sdk 	-I$(srcdir)/../../../core 	-I$(srcdir)/../../../kritacolor/ 	-I$(srcdir)/../../../ui 	-I$(top_srcdir)/kexi/scriptingcore/main 	$(KOFFICE_INCLUDES) 	$(all_includes)
#METASOURCES = AUTO
#SUBDIRS = python ruby
