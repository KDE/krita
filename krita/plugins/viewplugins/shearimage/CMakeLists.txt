kde4_header()




########### next target ###############

set(kritashearimage_PART_SRCS shearimage.cc dlg_shearimage.cc )

kde4_automoc(${kritashearimage_PART_SRCS})

kde4_add_ui3_files(kritashearimage_PART_SRCS wdg_shearimage.ui )

kde4_add_plugin(kritashearimage ${kritashearimage_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritashearimage )

target_link_libraries(kritashearimage kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritashearimage )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  shearimage.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritashearimage.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = shearimage.rc
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
#kde_module_LTLIBRARIES = kritashearimage.la
#
#kde_services_DATA = kritashearimage.desktop
#
#kritashearimage_la_SOURCES = wdg_shearimage.ui shearimage.cc dlg_shearimage.cc
#noinst_HEADERS = wdg_shearimage.h dlg_shearimage.h shearimage.h
#
#kritashearimage_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritashearimage_la_LIBADD =  ../../../libkritacommon.la
#
#kritashearimage_la_METASOURCES = AUTO
