
set(kritatoolperspectivetransform_PART_SRCS tool_perspectivetransform.cc kis_tool_perspectivetransform.cc kis_perspectivetransform_worker.cpp)

kde4_automoc(${kritatoolperspectivetransform_PART_SRCS})

kde4_add_plugin(kritatoolperspectivetransform ${kritatoolperspectivetransform_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolperspectivetransform )

target_link_libraries(kritatoolperspectivetransform  ${KDE4_KDECORE_LIBS} kritaui )

install(TARGETS kritatoolperspectivetransform DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  tool_perspectivetransform.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics )
install( FILES  kritatoolperspectivetransform.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#kde_services_DATA = kritatoolperspectivetransform.desktop
#
## all_includes must remain last!
#INCLUDES =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritatoolperspectivetransform_la_SOURCES = \
#	tool_perspectivetransform.cc \
#	kis_tool_perspectivetransform.cc 
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolperspectivetransform.la
#
#noinst_HEADERS = \
#	tool_perspectivetransform.h \
#	kis_tool_perspectivetransform.h 
#
#kritatoolperspectivetransform_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolperspectivetransform_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolperspectivetransform_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_perspectivetransform.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
