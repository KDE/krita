

set(kritatooltransform_PART_SRCS
    tool_transform.cc
    kis_tool_transform.cc
    )

kde4_automoc(${kritatooltransform_PART_SRCS})

kde4_add_ui_files(kritatooltransform_PART_SRCS wdg_tool_transform.ui)

kde4_add_plugin(kritatooltransform ${kritatooltransform_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatooltransform )

target_link_libraries(kritatooltransform kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritatooltransform )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  rotate_cursor.xpm )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritatooltransform.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritatooltransform.desktop
#
## all_includes must remain last!
#INCLUDES =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritatooltransform_la_SOURCES = \
#	wdg_tool_transform.ui \
#	tool_transform.cc \
#	kis_tool_transform.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatooltransform.la
#
#noinst_HEADERS = \
#	tool_transform.h \
#	kis_tool_transform.h
#
#kritatooltransform_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatooltransform_la_LIBADD =  ../../../libkritacommon.la
#
#kritatooltransform_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	rotate_cursor.xpm
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
