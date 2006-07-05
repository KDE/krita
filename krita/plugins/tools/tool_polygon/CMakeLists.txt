

set(kritatoolpolygon_PART_SRCS
    tool_polygon.cc
    kis_tool_polygon.cc
    )

kde4_automoc(${kritatoolpolygon_PART_SRCS})

kde4_add_plugin(kritatoolpolygon ${kritatoolpolygon_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolpolygon )

target_link_libraries(kritatoolpolygon kritaui)

install(TARGETS kritatoolpolygon  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polygon_cursor.png  	tool_polygon.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolpolygon.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kde_services_DATA = kritatoolpolygon.desktop
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
#kritatoolpolygon_la_SOURCES = \
#        tool_polygon.cc \
#	kis_tool_polygon.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolpolygon.la
#
#noinst_HEADERS = \
#	tool_polygon.h \
#	kis_tool_polygon.h
#
#kritatoolpolygon_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolpolygon_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolpolygon_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_polygon_cursor.png \
#	tool_polygon.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
