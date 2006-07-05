
set(kritacolorrange_PART_SRCS colorrange.cc dlg_colorrange.cc )

kde4_automoc(${kritacolorrange_PART_SRCS})

kde4_add_ui_files(kritacolorrange_PART_SRCS wdg_colorrange.ui )

kde4_add_plugin(kritacolorrange ${kritacolorrange_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorrange )

target_link_libraries(kritacolorrange kritaui)

install(TARGETS kritacolorrange  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  colorrange.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritacolorrange.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritacolorrange.la
#
#kritacolorrange_la_SOURCES = colorrange.cc dlg_colorrange.cc wdg_colorrange.ui
#noinst_HEADERS = wdg_colorrange.h dlg_colorrange.h colorrange.h
#
#kritacolorrange_la_LIBADD =  ../../../libkritacommon.la
#kritacolorrange_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = colorrange.rc
#EXTRA_DIST = $(kritarc_DATA)
#
#kde_services_DATA = kritacolorrange.desktop
#
#kritacolorrange_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
