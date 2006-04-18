kde4_header()

set(kritatoolcrop_PART_SRCS
    tool_crop.cc
    kis_tool_crop.cc
    )

kde4_automoc(${kritatoolcrop_PART_SRCS})

kde4_add_ui3_files(kritatoolcrop_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kritatoolcrop ${kritatoolcrop_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolcrop )

target_link_libraries(kritatoolcrop  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${PLUGIN_INSTALL_DIR} kritatoolcrop )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  tool_crop_cursor.png )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritatoolcrop.desktop )

kde4_footer()



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
