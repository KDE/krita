

set(kritatoolcrop_PART_SRCS
    tool_crop.cc
    kis_tool_crop.cc
    )

kde4_automoc(${kritatoolcrop_PART_SRCS})

kde4_add_ui_files(kritatoolcrop_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kritatoolcrop ${kritatoolcrop_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolcrop )

target_link_libraries(kritatoolcrop kritaui)

install(TARGETS kritatoolcrop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_crop_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolcrop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kde_services_DATA = kritatoolcrop.desktop
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
#kritatoolcrop_la_SOURCES = \
#	wdg_tool_crop.ui \
#	tool_crop.cc \
#	kis_tool_crop.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritatoolcrop.la
#
#noinst_HEADERS = \
#	tool_crop.h \
#	kis_tool_crop.h
#
#kritatoolcrop_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolcrop_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolcrop_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
#kritapics_DATA = \
#	tool_crop_cursor.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
