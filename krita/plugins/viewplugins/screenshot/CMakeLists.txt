




########### next target ###############

set(kritascreenshot_PART_SRCS screenshot.cpp ksnapshot.cpp regiongrabber.cpp )

kde4_automoc(${kritascreenshot_PART_SRCS})

kde4_add_ui3_files(kritascreenshot_PART_SRCS ksnapshotwidget.ui )

kde4_add_plugin(kritascreenshot ${kritascreenshot_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritascreenshot )

target_link_libraries(kritascreenshot kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritascreenshot )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  screenshot-krita.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritascreenshot.desktop )





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = screenshot-krita.rc
#
#EXTRA_DIST =  $(kritarc_DATA)
#
#INCLUDES =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritascreenshot.la
#
#kritascreenshot_la_SOURCES = screenshot.cpp ksnapshot.cpp regiongrabber.cpp ksnapshotwidget.ui
#noinst_HEADERS = screenshot.h ksnapshot.h regiongrabber.h ksnapshotwidget.h ksnapshotwidget.ui.h
#
#kritascreenshot_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritascreenshot_la_LIBADD =  ../../../libkritacommon.la
#
#kde_services_DATA = kritascreenshot.desktop
#
#kritascreenshot_la_METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kscreenshot_plugin.pot
