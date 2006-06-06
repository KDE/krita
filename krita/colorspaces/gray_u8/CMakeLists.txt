

add_subdirectory( templates )

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############

set(kritagrayscale_LIB_SRCS kis_gray_colorspace.cc )

kde4_automoc(${kritagrayscale_LIB_SRCS})

kde4_add_library(kritagrayscale SHARED ${kritagrayscale_LIB_SRCS})

target_link_libraries(kritagrayscale  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(kritagrayscale PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kritagrayscale )


########### next target ###############

set(kritagrayplugin_PART_SRCS gray_plugin.cc )

kde4_automoc(${kritagrayplugin_PART_SRCS})

kde4_add_plugin(kritagrayplugin ${kritagrayplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritagrayplugin )

target_link_libraries(kritagrayplugin  ${KDE4_KDECORE_LIBS} kritagrayscale kritacolor )

install_targets(${PLUGIN_INSTALL_DIR} kritagrayplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritagrayplugin.desktop )





#original Makefile.am contents follow:

#kde_services_DATA = kritagrayplugin.desktop
#
#INCLUDES  =  -I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../kritacolor/ \
#	-I$(interfacedir) \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#lib_LTLIBRARIES = libkritagrayscale.la
#libkritagrayscale_la_SOURCES = kis_gray_colorspace.cc
#libkritagrayscale_la_LDFLAGS = $(all_libraries)
#libkritagrayscale_la_LIBADD =  ../../kritacolor/libkritacolor.la
#
#kde_module_LTLIBRARIES = kritagrayplugin.la
#
#kritagrayplugin_la_SOURCES = gray_plugin.cc
#noinst_HEADERS = gray_plugin.h kis_gray_colorspace.h
#
#kritagrayplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritagrayplugin_la_LIBADD =  libkritagrayscale.la ../../kritacolor/libkritacolor.la
#
#kritagrayplugin_la_METASOURCES = AUTO
#
#if include_kunittest_tests
#TESTSDIR = tests
#endif
#
#SUBDIRS = . templates $(TESTSDIR)
#
