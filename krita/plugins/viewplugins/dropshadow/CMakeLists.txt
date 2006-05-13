




########### next target ###############

set(kritadropshadow_PART_SRCS kis_dropshadow.cc dlg_dropshadow.cc kis_dropshadow_plugin.cc )

kde4_automoc(${kritadropshadow_PART_SRCS})

kde4_add_ui3_files(kritadropshadow_PART_SRCS wdg_dropshadow.ui )

kde4_add_plugin(kritadropshadow ${kritadropshadow_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadropshadow )

target_link_libraries(kritadropshadow kritaui kritargb)

install_targets(${PLUGIN_INSTALL_DIR} kritadropshadow )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  dropshadow.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritadropshadow.desktop )





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = dropshadow.rc
#EXTRA_DIST = $(kritarc_DATA)
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../colorspaces/rgb_u8 \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#kde_module_LTLIBRARIES = kritadropshadow.la
#
#kritadropshadow_la_SOURCES = wdg_dropshadow.ui \
#	kis_dropshadow.cc dlg_dropshadow.cc  \
#	kis_dropshadow_plugin.cc
#
#noinst_HEADERS = wdg_dropshadow.h kis_dropshadow_plugin.h \
#		kis_dropshadow.h dlg_dropshadow.h
#
#kritadropshadow_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritadropshadow_la_LIBADD =  ../../../libkritacommon.la ../../../colorspaces/rgb_u8/libkritargb.la
#
#kde_services_DATA = kritadropshadow.desktop
#
#kritadropshadow_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
