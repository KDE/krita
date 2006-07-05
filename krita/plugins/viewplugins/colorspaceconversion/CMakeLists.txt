
set(kritacolorspaceconversion_PART_SRCS colorspaceconversion.cc dlg_colorspaceconversion.cc )

kde4_automoc(${kritacolorspaceconversion_PART_SRCS})

kde4_add_ui_files(kritacolorspaceconversion_PART_SRCS wdgconvertcolorspace.ui )

kde4_add_plugin(kritacolorspaceconversion ${kritacolorspaceconversion_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorspaceconversion )

target_link_libraries(kritacolorspaceconversion kritaui)

install(TARGETS kritacolorspaceconversion  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  colorspaceconversion.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritacolorspaceconversion.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = colorspaceconversion.rc
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
#
#kde_module_LTLIBRARIES = kritacolorspaceconversion.la
#
#kritacolorspaceconversion_la_SOURCES = wdgconvertcolorspace.ui colorspaceconversion.cc dlg_colorspaceconversion.cc
#noinst_HEADERS = wdgconvertcolorspace.h dlg_colorspaceconversion.h colorspaceconversion.h
#
#kritacolorspaceconversion_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacolorspaceconversion_la_LIBADD =  ../../../libkritacommon.la
#
#kritacolorspaceconversion_la_METASOURCES = AUTO
#
#kde_services_DATA = kritacolorspaceconversion.desktop
#
