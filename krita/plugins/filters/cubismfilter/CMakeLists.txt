kde4_header()




########### next target ###############

set(kritacubismfilter_PART_SRCS kis_cubism_filter_plugin.cc kis_cubism_filter.cc kis_polygon.cc )

kde4_automoc(${kritacubismfilter_PART_SRCS})

kde4_add_plugin(kritacubismfilter ${kritacubismfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacubismfilter )

target_link_libraries(kritacubismfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritacubismfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritacubismfilter.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritacubismfilter.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritacubismfilter.la
#
#kritacubismfilter_la_SOURCES = kis_cubism_filter_plugin.cc \
#	kis_cubism_filter.cc \
#        kis_polygon.cc
#
#noinst_HEADERS = kis_cubism_filter_plugin.h \
#	kis_cubism_filter.h \
#        kis_polygon.h
#
#kritacubismfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacubismfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritacubismfilter_la_METASOURCES = AUTO
