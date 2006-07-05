

include_directories( ${OPENEXR_INCLUDE_DIR})
if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)


########### next target ###############

set(krita_rgb_f16half_LIB_SRCS kis_rgb_f16half_colorspace.cc )

kde4_automoc(${krita_rgb_f16half_LIB_SRCS})

kde4_add_library(krita_rgb_f16half SHARED ${krita_rgb_f16half_LIB_SRCS})

target_link_libraries(krita_rgb_f16half  ${KDE4_KDECORE_LIBS} kritacolor ${OPENEXR_LIBRARIES})

set_target_properties(krita_rgb_f16half PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krita_rgb_f16half  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(krita_rgb_f16half_plugin_PART_SRCS rgb_f16half_plugin.cc )

kde4_automoc(${krita_rgb_f16half_plugin_PART_SRCS})

kde4_add_plugin(krita_rgb_f16half_plugin ${krita_rgb_f16half_plugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krita_rgb_f16half_plugin )

target_link_libraries(krita_rgb_f16half_plugin  ${KDE4_KDECORE_LIBS} krita_rgb_f16half kritacolor  ${OPENEXR_LIBRARIES})

install(TARGETS krita_rgb_f16half_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_rgb_f16half_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

## Install the desktop file needed to detect the plugin
#kde_services_DATA = krita_rgb_f16half_plugin.desktop
#
#INCLUDES  = \
#	-I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../kritacolor/ \
#	$(OPENEXR_CFLAGS) \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#lib_LTLIBRARIES = libkrita_rgb_f16half.la
#libkrita_rgb_f16half_la_SOURCES = kis_rgb_f16half_colorspace.cc
#libkrita_rgb_f16half_la_LDFLAGS = $(all_libraries)
#libkrita_rgb_f16half_la_LIBADD =  ../../kritacolor/libkritacolor.la
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = krita_rgb_f16half_plugin.la
#
## Srcs for the plugin
#krita_rgb_f16half_plugin_la_SOURCES = rgb_f16half_plugin.cc
#noinst_HEADERS = rgb_f16half_plugin.h kis_rgb_f16half_colorspace.h
#
#krita_rgb_f16half_plugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#krita_rgb_f16half_plugin_la_LIBADD = libkrita_rgb_f16half.la ../../kritacolor/libkritacolor.la
#
#krita_rgb_f16half_plugin_la_METASOURCES = AUTO
#
#if include_kunittest_tests
#TESTSDIR = tests
#endif
#
#SUBDIRS = . $(TESTSDIR)
#
