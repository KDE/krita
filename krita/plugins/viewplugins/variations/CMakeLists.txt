kde4_header()




########### next target ###############

set(kritavariations_PART_SRCS variations.cc dlg_variations.cc )

kde4_automoc(${kritavariations_PART_SRCS})

kde4_add_ui3_files(kritavariations_PART_SRCS wdg_variations.ui )

kde4_add_plugin(kritavariations ${kritavariations_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritavariations )

target_link_libraries(kritavariations  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${PLUGIN_INSTALL_DIR} kritavariations )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  variations.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritavariations.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = variations.rc
#
#EXTRA_DIST = $(kritarc_DATA)
#
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritavariations.la
#
#kritavariations_la_SOURCES = variations.cc dlg_variations.cc wdg_variations.ui 
#noinst_HEADERS = wdg_variations.h
#
#kritavariations_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritavariations_la_LIBADD =  ../../../libkritacommon.la
#
#kde_services_DATA = kritavariations.desktop
#
#kritavariations_la_METASOURCES = AUTO
