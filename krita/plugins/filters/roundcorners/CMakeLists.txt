




########### next target ###############

set(kritaroundcornersfilter_PART_SRCS kis_round_corners_filter_plugin.cc kis_round_corners_filter.cc )

kde4_automoc(${kritaroundcornersfilter_PART_SRCS})

kde4_add_plugin(kritaroundcornersfilter ${kritaroundcornersfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaroundcornersfilter )

target_link_libraries(kritaroundcornersfilter kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritaroundcornersfilter )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritaroundcornersfilter.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritaroundcornersfilter.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritaroundcornersfilter.la
#
#kritaroundcornersfilter_la_SOURCES = kis_round_corners_filter_plugin.cc \
#	kis_round_corners_filter.cc
#
#noinst_HEADERS = kis_round_corners_filter_plugin.h \
#	kis_round_corners_filter.h
#
#kritaroundcornersfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaroundcornersfilter_la_LIBADD =  ../../../libkritacommon.la
#
#kritaroundcornersfilter_la_METASOURCES = AUTO
