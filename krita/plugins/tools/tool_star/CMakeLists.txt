

set(kritatoolstar_PART_SRCS
    tool_star.cc
    kis_tool_star.cc
    )

kde4_automoc(${kritatoolstar_PART_SRCS})

kde4_add_ui_files(kritatoolstar_PART_SRCS wdg_tool_star.ui)

kde4_add_plugin(kritatoolstar ${kritatoolstar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolstar )

target_link_libraries(kritatoolstar kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritatoolstar )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  tool_star.png  	tool_star_cursor.png )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritatoolstar.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritatoolstar.desktop
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
#kritatoolstar_la_SOURCES = \
#	wdg_tool_star.ui \
#        tool_star.cc \
#	kis_tool_star.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolstar.la
#
#noinst_HEADERS = \
#	tool_star.h \
#	kis_tool_star.h
#
#kritatoolstar_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolstar_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolstar_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_star.png \
#	tool_star_cursor.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
