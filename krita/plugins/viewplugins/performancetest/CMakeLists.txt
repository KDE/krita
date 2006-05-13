




########### next target ###############

set(kritaperftest_PART_SRCS perftest.cc dlg_perftest.cc )

kde4_automoc(${kritaperftest_PART_SRCS})

kde4_add_ui3_files(kritaperftest_PART_SRCS wdg_perftest.ui )

kde4_add_plugin(kritaperftest ${kritaperftest_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaperftest )

target_link_libraries(kritaperftest kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritaperftest )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  perftest.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritaperftest.desktop )





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = perftest.rc
#
#EXTRA_DIST = $(kritarc_DATA)
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritaperftest.la
#
#kritaperftest_la_SOURCES = wdg_perftest.ui perftest.cc dlg_perftest.cc
#noinst_HEADERS = wdg_perftest.h dlg_perftest.h perftest.h
#
#kritaperftest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaperftest_la_LIBADD =  ../../../libkritacommon.la
#
#kde_services_DATA = kritaperftest.desktop
#
#kritaperftest_la_METASOURCES = AUTO
