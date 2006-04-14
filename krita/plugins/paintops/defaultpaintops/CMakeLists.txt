kde4_header()

set(kritadefaultpaintops_PART_SRCS 
	defaultpaintops_plugin.cc
	kis_airbrushop.cc
	kis_brushop.cc
	kis_duplicateop.cc
	kis_eraseop.cc
	kis_penop.cc
    )

kde4_automoc(${kritadefaultpaintops_PART_SRCS})

kde4_add_plugin(kritadefaultpaintops ${kritadefaultpaintops_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadefaultpaintops )

target_link_libraries(kritadefaultpaintops  ${KDE4_KDECORE_LIBS} kritacommon kopainter kofficecore )

install_targets(${LIB_INSTALL_DIR}/kde4 kritadefaultpaintops )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritadefaultpaintops.desktop )
install_files( ${DATA_INSTALL_DIR}/krita/images FILES 
    airbrush.png 
    paintbrush.png 
    eraser.png 
    pencil.png )

kde4_footer()



#original Makefile.am contents follow:

#kritaimagesdir = $(prefix)/share/apps/krita/images
#
#kritaimages_DATA = \
#	airbrush.png \
#	paintbrush.png \
#	eraser.png \
#	pencil.png
#
#
#kde_services_DATA = kritadefaultpaintops.desktop
#
## all_includes must remain last!
#INCLUDES = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)
#
#
#kritadefaultpaintops_la_SOURCES = \
#	defaultpaintops_plugin.cc \
#	kis_airbrushop.cc \
#	kis_brushop.cc \
#	kis_duplicateop.cc \
#	kis_eraseop.cc \
#	kis_penop.cc
#
#noinst_HEADERS = defaultpaintops_plugin.h 	kis_airbrushop.h 	kis_brushop.h \
#		kis_duplicateop.h 	kis_eraseop.h 	kis_penop.h
#
#kde_module_LTLIBRARIES = kritadefaultpaintops.la
#
#kritadefaultpaintops_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritadefaultpaintops_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)
#
#kritadefaultpaintops_la_METASOURCES = AUTO
