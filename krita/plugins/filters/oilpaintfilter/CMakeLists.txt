kde4_header()




########### next target ###############

set(kritaoilpaintfilter_PART_SRCS kis_oilpaint_filter_plugin.cc kis_oilpaint_filter.cc )

kde4_automoc(${kritaoilpaintfilter_PART_SRCS})

kde4_add_plugin(kritaoilpaintfilter ${kritaoilpaintfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaoilpaintfilter )

target_link_libraries(kritaoilpaintfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritaoilpaintfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritaoilpaintfilter.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritaoilpaintfilter.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../ui \
#	-I../../../ui/ \
#        -I$(srcdir)/../../../../lib/kofficecore \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritaoilpaintfilter.la
#
#kritaoilpaintfilter_la_SOURCES = kis_oilpaint_filter_plugin.cc \
#	kis_oilpaint_filter.cc
#
#noinst_HEADERS = kis_oilpaint_filter_plugin.h \
#	kis_oilpaint_filter.h
#
#kritaoilpaintfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaoilpaintfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritaoilpaintfilter_la_METASOURCES = AUTO
