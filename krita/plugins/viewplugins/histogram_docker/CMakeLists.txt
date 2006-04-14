kde4_header()

set(kritahistogramdocker_PART_SRCS
    histogramdocker.cc
    kis_imagerasteredcache.cc
    kis_cachedhistogram.cc
    kis_accumulating_producer.cc
    )

kde4_automoc(${kritahistogramdocker_PART_SRCS})

kde4_add_plugin(kritahistogramdocker ${kritahistogramdocker_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistogramdocker )

target_link_libraries(kritahistogramdocker  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kritahistogramdocker )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  kritahistogramdocker.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritahistogramdocker.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritahistogramdocker.desktop
#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = kritahistogramdocker.rc
#
#INCLUDES  = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritahistogramdocker_la_SOURCES = histogramdocker.cc  kis_imagerasteredcache.cc kis_cachedhistogram.cc kis_accumulating_producer.cc
#
#kde_module_LTLIBRARIES = kritahistogramdocker.la
#noinst_HEADERS = histogramdocker.h kis_imagerasteredcache.h kis_cachedhistogram.h kis_accumulating_producer.h
#
#kritahistogramdocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritahistogramdocker_la_LIBADD =  ../../../libkritacommon.la
#
#kritahistogramdocker_la_METASOURCES = AUTO
