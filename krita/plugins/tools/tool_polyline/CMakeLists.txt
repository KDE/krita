

set(kritatoolpolyline_PART_SRCS
    tool_polyline.cc
    kis_tool_polyline.cc
    )

kde4_automoc(${kritatoolpolyline_PART_SRCS})

kde4_add_plugin(kritatoolpolyline ${kritatoolpolyline_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolpolyline )

target_link_libraries(kritatoolpolyline kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritatoolpolyline )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  tool_polyline_cursor.png  	polyline.png )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritatoolpolyline.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritatoolpolyline.desktop
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
#kritatoolpolyline_la_SOURCES = \
#        tool_polyline.cc \
#	kis_tool_polyline.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolpolyline.la
#
#noinst_HEADERS = \
#	tool_polyline.h \
#	kis_tool_polyline.h
#
#kritatoolpolyline_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolpolyline_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolpolyline_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_polyline_cursor.png \
#	polyline.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
