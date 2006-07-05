




########### next target ###############

set(kritawetplugin_PART_SRCS
   kis_wet_colorspace.cc
   wet_plugin.cc
   kis_wetop.cc
   kis_wet_palette_widget.cc
   kis_wetness_visualisation_filter.cc
   kis_texture_painter.cc
   kis_texture_filter.cc
   wetphysicsfilter.cc )

kde4_automoc(${kritawetplugin_PART_SRCS})

kde4_add_ui_files(kritawetplugin_PART_SRCS wdgpressure.ui )

kde4_add_plugin(kritawetplugin ${kritawetplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritawetplugin )

target_link_libraries(kritawetplugin kritaui)

install(TARGETS kritawetplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  wetplugin.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritawetplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = wetplugin.rc
#kde_services_DATA = kritawetplugin.desktop
#
#EXTRA_DIST = $(kritarc_DATA)
#
#INCLUDES  = -I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../core \
#	-I$(srcdir)/../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../kritacolor/ \
#	-I$(srcdir)/../../ui \
#	-I$(srcdir)/../../kopalette \
#	$(KOFFICE_INCLUDES) \
#	-I$(interfacedir) \
#	$(KOPAINTER_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritawetplugin.la
#
#kritawetplugin_la_SOURCES = kis_wet_colorspace.cc wet_plugin.cc kis_wetop.cc kis_wet_palette_widget.cc kis_wetness_visualisation_filter.cc kis_texture_painter.cc kis_texture_filter.cc wetphysicsfilter.cc wdgpressure.ui
#noinst_HEADERS = kis_wet_colorspace.h wet_plugin.h wetphysicsfilter.h kis_wetop.cc kis_wet_palette_widget.h kis_texture_painter.h kis_wetness_visualisation_filter.h kis_texture_filter.h wetphysicsfilter.h
#
#kritawetplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritawetplugin_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)
#
#kritawetplugin_la_METASOURCES = AUTO
#
