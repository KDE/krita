




########### next target ###############

set(kritapixelizefilter_PART_SRCS kis_pixelize_filter_plugin.cc kis_pixelize_filter.cc )

kde4_automoc(${kritapixelizefilter_PART_SRCS})

kde4_add_plugin(kritapixelizefilter ${kritapixelizefilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritapixelizefilter )

target_link_libraries(kritapixelizefilter kritaui)

install(TARGETS kritapixelizefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapixelizefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kde_services_DATA = kritapixelizefilter.desktop
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
#kde_module_LTLIBRARIES = kritapixelizefilter.la
#
#kritapixelizefilter_la_SOURCES = kis_pixelize_filter_plugin.cc \
#	kis_pixelize_filter.cc
#
#noinst_HEADERS = kis_pixelize_filter_plugin.h \
#	kis_pixelize_filter.h
#
#kritapixelizefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritapixelizefilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritapixelizefilter_la_METASOURCES = AUTO
