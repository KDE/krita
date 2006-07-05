

set(kritaexample_PART_SRCS example.cc )

kde4_automoc(${kritaexample_PART_SRCS})

kde4_add_plugin(kritaexample ${kritaexample_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaexample )

target_link_libraries(kritaexample kritaui)

install(TARGETS kritaexample  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES   DESTINATION ${DATA_INSTALL_DIR}/krita/kritaplugins)
install( FILES  kritaexample.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/krita/kritaplugins
#
#kde_services_DATA = kritaexample.desktop
#
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritaexample_la_SOURCES = example.cc
#
#kde_module_LTLIBRARIES = kritaexample.la
#noinst_HEADERS = example.h
#
#kritaexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaexample_la_LIBADD =  ../../../libkritacommon.la
#
#kritaexample_la_METASOURCES = AUTO
