

add_subdirectory( templates )




########### next target ###############

set(kritacmykplugin_PART_SRCS cmyk_plugin.cc kis_cmyk_colorspace.cc )

kde4_automoc(${kritacmykplugin_PART_SRCS})

kde4_add_plugin(kritacmykplugin ${kritacmykplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacmykplugin )

target_link_libraries(kritacmykplugin  ${KDE4_KDECORE_LIBS} kritacolor )

install(TARGETS kritacmykplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacmykplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kde_services_DATA = kritacmykplugin.desktop
#
#INCLUDES  = -I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor/color_strategy \
#	-I$(srcdir)/../../kritacolor \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritacmykplugin.la
#
#kritacmykplugin_la_SOURCES = cmyk_plugin.cc kis_cmyk_colorspace.cc
#noinst_HEADERS = cmyk_plugin.h kis_cmyk_colorspace.h 
#
#kritacmykplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacmykplugin_la_LIBADD =  ../../kritacolor/libkritacolor.la
#
#kritacmykplugin_la_METASOURCES = AUTO
#
#
#SUBDIRS=templates
