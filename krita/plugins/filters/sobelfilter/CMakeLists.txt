kde4_header()




########### next target ###############

set(kritasobelfilter_PART_SRCS kis_sobel_filter_plugin.cc kis_sobel_filter.cc )

kde4_automoc(${kritasobelfilter_PART_SRCS})

kde4_add_plugin(kritasobelfilter ${kritasobelfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritasobelfilter )

target_link_libraries(kritasobelfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritasobelfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritasobelfilter.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritasobelfilter.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritasobelfilter.la
#
#kritasobelfilter_la_SOURCES = kis_sobel_filter_plugin.cc \
#	kis_sobel_filter.cc
#
#noinst_HEADERS = kis_sobel_filter_plugin.h \
#	kis_sobel_filter.h
#
#kritasobelfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritasobelfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritasobelfilter_la_METASOURCES = AUTO
