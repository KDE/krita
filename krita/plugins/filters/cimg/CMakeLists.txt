kde4_header()

set(kritacimg_PART_SRCS
    kis_cimg_filter.cc
    kis_cimg_plugin.cc
    kis_cimgconfig_widget.cc
    )

kde4_automoc(${kritacimg_PART_SRCS})

kde4_add_ui_files(kritacimg_PART_SRCS wdg_cimg.ui)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

kde4_add_plugin(kritacimg ${kritacimg_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacimg )

target_link_libraries(kritacimg kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritacimg )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritacimg.desktop )

kde4_footer()



#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#kde_services_DATA = kritacimg.desktop
#
## all_includes must remain last!
#INCLUDES =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritacimg_la_SOURCES = \
#	wdg_cimg.ui\
#	kis_cimg_filter.cc\
#	kis_cimg_plugin.cc\
#	kis_cimgconfig_widget.cc
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kritacimg.la
#
#noinst_HEADERS = \
#	CImg.h\
#	kis_cimg_filter.h\
#	kis_cimg_plugin.h\
#	kis_cimgconfig_widget.h
#
#kritacimg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacimg_la_LIBADD =  ../../../libkritacommon.la
#
#kritacimg_la_METASOURCES = AUTO
#
#
#KDE_OPTIONS = nofinal
