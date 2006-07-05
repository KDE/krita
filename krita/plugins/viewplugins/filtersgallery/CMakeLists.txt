

set(kritafiltersgallery_PART_SRCS
    filters_gallery.cc
    kis_dlg_filtersgallery.cc
    )

kde4_automoc(${kritafiltersgallery_PART_SRCS})

kde4_add_ui_files(kritafiltersgallery_PART_SRCS kis_wdg_filtersgallery.ui)

kde4_add_plugin(kritafiltersgallery ${kritafiltersgallery_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritafiltersgallery )

target_link_libraries(kritafiltersgallery kritaui)

install(TARGETS kritafiltersgallery  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafiltersgallery.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritafiltersgallery.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = kritafiltersgallery.rc
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
#kritafiltersgallery_la_SOURCES = filters_gallery.cc \
#		kis_dlg_filtersgallery.cc kis_wdg_filtersgallery.ui
#
#kde_module_LTLIBRARIES = kritafiltersgallery.la
#noinst_HEADERS = filters_gallery.h kis_dlg_filtersgallery.h
#
#kde_services_DATA = kritafiltersgallery.desktop
#
#kritafiltersgallery_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritafiltersgallery_la_LIBADD =  ../../../libkritacommon.la
#
#kritafiltersgallery_la_METASOURCES = AUTO
