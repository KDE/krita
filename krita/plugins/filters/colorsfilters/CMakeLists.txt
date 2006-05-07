kde4_header()

set(kritacolorsfilters_PART_SRCS
    colorsfilters.cc
    kis_perchannel_filter.cc
    kis_brightness_contrast_filter.cc
    )

kde4_automoc(${kritacolorsfilters_PART_SRCS})

kde4_add_ui_files(kritacolorsfilters_PART_SRCS
    wdg_perchannel.ui
    wdg_brightness_contrast.ui
    )

kde4_add_plugin(kritacolorsfilters ${kritacolorsfilters_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorsfilters )

target_link_libraries(kritacolorsfilters  ${KDE4_KDECORE_LIBS} kritacommon)

install_targets(${PLUGIN_INSTALL_DIR} kritacolorsfilters )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritacolorsfilter.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritacolorsfilter.desktop
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritacolorsfilters_la_SOURCES = colorsfilters.cc \
#	kis_perchannel_filter.cc \
#	wdg_perchannel.ui \
#	wdg_brightness_contrast.ui \
#	kis_brightness_contrast_filter.cc
#
#noinst_HEADERS = colorsfilters.h \
#	kis_perchannel_filter.h \
#	kis_brightness_contrast_filter.h
#
#kritacolorsfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacolorsfilters_la_LIBADD =  ../../../libkritacommon.la
#
#kde_module_LTLIBRARIES = kritacolorsfilters.la
#
#kritacolorsfilters_la_METASOURCES = AUTO
