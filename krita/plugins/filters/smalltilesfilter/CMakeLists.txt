kde4_header()




########### next target ###############

set(kritasmalltilesfilter_PART_SRCS kis_small_tiles_filter_plugin.cc kis_small_tiles_filter.cc )

kde4_automoc(${kritasmalltilesfilter_PART_SRCS})

kde4_add_plugin(kritasmalltilesfilter ${kritasmalltilesfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritasmalltilesfilter )

target_link_libraries(kritasmalltilesfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritasmalltilesfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritasmalltilesfilter.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritasmalltilesfilter.desktop
#
#INCLUDES  = \
#	-I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../ui \
#        -I$(srcdir)/../../../../lib/kofficecore \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritasmalltilesfilter.la
#
#kritasmalltilesfilter_la_SOURCES = kis_small_tiles_filter_plugin.cc \
#	kis_small_tiles_filter.cc
#
#noinst_HEADERS = kis_small_tiles_filter_plugin.h \
#	kis_small_tiles_filter.h
#
#kritasmalltilesfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritasmalltilesfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritasmalltilesfilter_la_METASOURCES = AUTO
