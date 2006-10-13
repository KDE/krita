########### next target ###############

set(kritacpaintbrush_PART_SRCS bristle.cpp brush.cpp gauss.cpp sample.cpp stroke.cpp cpaintbrush.cc kis_cpaintop.cc )

kde4_add_ui3_files(kritacpaintbrush_PART_SRCS  wdgcpaintoptions.ui)

kde4_automoc(kritacpaintbrush ${kritacpaintbrush_PART_SRCS})

kde4_add_plugin(kritacpaintbrush ${kritacpaintbrush_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacpaintbrush )

target_link_libraries(kritacpaintbrush  ${KDE4_KDECORE_LIBS} kritacommon kopainter kofficecore )

install(TARGETS kritacpaintbrush DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kritacpaintbrush.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#kde_services_DATA = kritacpaintbrush.desktop
#
## all_includes must remain last!
#INCLUDES = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)
#
#kritacpaintbrush_la_SOURCES = \
#	bristle.cpp \
#	brush.cpp \
#	gauss.cpp \
#	sample.cpp \
#	stroke.cpp \
#	cpaintbrush.cc \
#	kis_cpaintop.cc \
#	wdgcpaintoptions.ui
#
#noinst_HEADERS = \
#	bristle.h \
#	brush.h \
#	cpaintbrush.h \
#	gauss.h \
#	kis_cpaintop.h \
#	sample.h \
#	stroke.h
#
#kde_module_LTLIBRARIES = kritacpaintbrush.la
#
#kritacpaintbrush_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritacpaintbrush_la_LIBADD =  ../../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)
#
#kritacpaintbrush_la_METASOURCES = AUTO
