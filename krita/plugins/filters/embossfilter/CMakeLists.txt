




########### next target ###############

set(kritaembossfilter_PART_SRCS kis_emboss_filter_plugin.cc kis_emboss_filter.cc )

kde4_automoc(${kritaembossfilter_PART_SRCS})

kde4_add_plugin(kritaembossfilter ${kritaembossfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaembossfilter )

target_link_libraries(kritaembossfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritaembossfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritaembossfilter.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritaembossfilter.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../ui \
#        -I$(srcdir)/../../../../lib/kofficecore \
#        -I$(srcdir)/../../../../lib/kofficeui \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritaembossfilter.la
#
#kritaembossfilter_la_SOURCES = kis_emboss_filter_plugin.cc \
#	kis_emboss_filter.cc
#
#noinst_HEADERS = kis_emboss_filter_plugin.h \
#        kis_emboss_filter.h
#
#kritaembossfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaembossfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritaembossfilter_la_METASOURCES = AUTO
