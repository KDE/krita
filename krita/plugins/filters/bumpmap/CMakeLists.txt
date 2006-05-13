

set(kritabumpmap_PART_SRCS
    bumpmap.cc
    )

kde4_automoc(${kritabumpmap_PART_SRCS})

kde4_add_ui_files(kritabumpmap_PART_SRCS wdgbumpmap.ui)

kde4_add_plugin(kritabumpmap ${kritabumpmap_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritabumpmap )

target_link_libraries(kritabumpmap kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritabumpmap )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritabumpmapfilter.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritabumpmapfilter.desktop
#
#INCLUDES  = -I$(srcdir)/../../../sdk \
#	 -I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$(srcdir)/../../../../lib/kofficecore \
#	$(all_includes)
#
#kritabumpmap_la_SOURCES = bumpmap.cc wdgbumpmap.ui
#
#kde_module_LTLIBRARIES = kritabumpmap.la
#noinst_HEADERS = bumpmap.h
#
#kritabumpmap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritabumpmap_la_LIBADD =  ../../../libkritacommon.la
#
#kritabumpmap_la_METASOURCES = AUTO
