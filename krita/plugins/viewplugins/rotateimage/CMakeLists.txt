




########### next target ###############

set(kritarotateimage_PART_SRCS rotateimage.cc dlg_rotateimage.cc )

kde4_automoc(${kritarotateimage_PART_SRCS})

kde4_add_ui_files(kritarotateimage_PART_SRCS wdg_rotateimage.ui )

kde4_add_plugin(kritarotateimage ${kritarotateimage_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritarotateimage )

target_link_libraries(kritarotateimage kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritarotateimage )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  rotateimage.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritarotateimage.desktop )





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = rotateimage.rc
#
#EXTRA_DIST = $(kritarc_DATA)
#
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritarotateimage.la
#
#kritarotateimage_la_SOURCES = wdg_rotateimage.ui rotateimage.cc dlg_rotateimage.cc
#noinst_HEADERS = wdg_rotateimage.h dlg_rotateimage.h rotateimage.h
#
#kritarotateimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritarotateimage_la_LIBADD =  ../../../libkritacommon.la
#
#kde_services_DATA = kritarotateimage.desktop
#
#kritarotateimage_la_METASOURCES = AUTO
