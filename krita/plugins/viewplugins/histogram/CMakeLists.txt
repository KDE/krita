




########### next target ###############

set(kritahistogram_PART_SRCS histogram.cc dlg_histogram.cc kis_histogram_widget.cc )

kde4_automoc(${kritahistogram_PART_SRCS})

kde4_add_ui3_files(kritahistogram_PART_SRCS wdghistogram.ui )

kde4_add_plugin(kritahistogram ${kritahistogram_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistogram )

target_link_libraries(kritahistogram kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritahistogram )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  histogram.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritahistogram.desktop )





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = histogram.rc
#
#EXTRA_DIST = $(kritarc_DATA)
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritahistogram.la
#
#kritahistogram_la_SOURCES = histogram.cc dlg_histogram.cc wdghistogram.ui kis_histogram_widget.cc
#noinst_HEADERS = dlg_histogram.h histogram.h wdghistogram.h kis_histogram_widget.h
#
#kde_services_DATA = kritahistogram.desktop
#
#kritahistogram_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritahistogram_la_LIBADD =  ../../../libkritacommon.la
#
#kritahistogram_la_METASOURCES = AUTO
