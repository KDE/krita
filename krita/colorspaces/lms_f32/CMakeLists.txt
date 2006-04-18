kde4_header()




########### next target ###############

set(krita_lms_f32_LIB_SRCS kis_lms_f32_colorspace.cc )

kde4_automoc(${krita_lms_f32_LIB_SRCS})

kde4_add_library(krita_lms_f32 SHARED ${krita_lms_f32_LIB_SRCS})

target_link_libraries(krita_lms_f32  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(krita_lms_f32 PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} krita_lms_f32 )


########### next target ###############

set(krita_lms_f32_plugin_PART_SRCS lms_f32_plugin.cc )

kde4_automoc(${krita_lms_f32_plugin_PART_SRCS})

kde4_add_plugin(krita_lms_f32_plugin ${krita_lms_f32_plugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krita_lms_f32_plugin )

target_link_libraries(krita_lms_f32_plugin  ${KDE4_KDECORE_LIBS} krita_lms_f32 kritacolor )

install_targets(${PLUGIN_INSTALL_DIR} krita_lms_f32_plugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  krita_lms_f32_plugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Install the desktop file needed to detect the plugin
#kde_services_DATA = krita_lms_f32_plugin.desktop
#
#INCLUDES  = \
#	-I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../kritacolor/ \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#lib_LTLIBRARIES = libkrita_lms_f32.la
#libkrita_lms_f32_la_SOURCES = kis_lms_f32_colorspace.cc
#libkrita_lms_f32_la_LDFLAGS = $(all_libraries)
#libkrita_lms_f32_la_LIBADD =  ../../kritacolor/libkritacolor.la
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = krita_lms_f32_plugin.la
#
## Srcs for the plugin
#krita_lms_f32_plugin_la_SOURCES = lms_f32_plugin.cc
#noinst_HEADERS = lms_f32_plugin.h kis_lms_f32_colorspace.h
#
#krita_lms_f32_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#krita_lms_f32_plugin_la_LIBADD =  libkrita_lms_f32.la ../../kritacolor/libkritacolor.la
#
#krita_lms_f32_plugin_la_METASOURCES = AUTO
#
