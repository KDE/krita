kde4_header()

set(kritamodifyselection_PART_SRCS
    dlg_grow_selection.cc
    dlg_shrink_selection.cc
    dlg_border_selection.cc
    modify_selection.cc
    )

kde4_automoc(${kritamodifyselection_PART_SRCS})

kde4_add_ui3_files(kritamodifyselection_PART_SRCS
    wdg_grow_selection.ui
    wdg_shrink_selection.ui
    wdg_border_selection.ui
    )

kde4_add_plugin(kritamodifyselection ${kritamodifyselection_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritamodifyselection )

target_link_libraries(kritamodifyselection kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritamodifyselection )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  modify_selection.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritamodifyselection.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = modify_selection.rc
#EXTRA_DIST = $(kritarc_DATA)
#
#kde_services_DATA = kritamodifyselection.desktop
#
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritamodifyselection_la_SOURCES = wdg_grow_selection.ui wdg_shrink_selection.ui wdg_border_selection.ui \
#dlg_grow_selection.cc dlg_shrink_selection.cc dlg_border_selection.cc modify_selection.cc
#
#noinst_HEADERS = wdg_grow_selection.h wdg_shrink_selection.h wdg_border_selection.h dlg_grow_selection.h \
#dlg_shrink_selection.h dlg_border_selection.h modify_selection.h
#
#kde_module_LTLIBRARIES = kritamodifyselection.la
#
#kritamodifyselection_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritamodifyselection_la_LIBADD =  ../../../libkritacommon.la
#
#kritamodifyselection_la_METASOURCES = AUTO
