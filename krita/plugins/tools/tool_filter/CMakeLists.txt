

set(kritatoolfilter_PART_SRCS
    kis_filterop.cc
    kis_tool_filter.cc
    tool_filter.cc
    )

kde4_automoc(${kritatoolfilter_PART_SRCS})

kde4_add_plugin(kritatoolfilter ${kritatoolfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolfilter )

target_link_libraries(kritatoolfilter kritaui)

install(TARGETS kritatoolfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_filter.png  	tool_filter_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kde_services_DATA = kritatoolfilter.desktop
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
#kritatoolfilter_la_SOURCES = \
#	kis_filterop.cc \
#	kis_tool_filter.cc \
#	tool_filter.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolfilter.la
#
#noinst_HEADERS = \
#	kis_filterop.h \
#	kis_tool_filter.h \
#	tool_filter.h
#
#kritatoolfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolfilter_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_filter.png \
#	tool_filter_cursor.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
