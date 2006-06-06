

add_subdirectory( templates )

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)



########### next target ###############

set(kritargb_LIB_SRCS kis_rgb_colorspace.cc )

kde4_automoc(${kritargb_LIB_SRCS})

kde4_add_library(kritargb SHARED ${kritargb_LIB_SRCS})

target_link_libraries(kritargb  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(kritargb PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kritargb )


########### next target ###############

set(kritargbplugin_PART_SRCS rgb_plugin.cc )

kde4_automoc(${kritargbplugin_PART_SRCS})

kde4_add_plugin(kritargbplugin ${kritargbplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritargbplugin )

target_link_libraries(kritargbplugin  ${KDE4_KDECORE_LIBS} kritargb kritacolor )

install_targets(${PLUGIN_INSTALL_DIR} kritargbplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritargbplugin.desktop )





#original Makefile.am contents follow:

## Install the desktop file needed to detect the plugin
#kde_services_DATA = kritargbplugin.desktop
#
#INCLUDES  =  -I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../kritacolor/ \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#lib_LTLIBRARIES = libkritargb.la
#libkritargb_la_SOURCES = kis_rgb_colorspace.cc
#libkritargb_la_LDFLAGS = $(all_libraries)
#libkritargb_la_LIBADD =  ../../kritacolor/libkritacolor.la
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritargbplugin.la
#
## Srcs for the plugin
#kritargbplugin_la_SOURCES = rgb_plugin.cc
#noinst_HEADERS = rgb_plugin.h kis_rgb_colorspace.h
#
#kritargbplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritargbplugin_la_LIBADD =  libkritargb.la ../../kritacolor/libkritacolor.la
#
#kritargbplugin_la_METASOURCES = AUTO
##METASOURCES = AUTO # XXX: which of the two?
#
#if include_kunittest_tests
#TESTSDIR = tests
#endif
#
#SUBDIRS = . templates $(TESTSDIR)
#
