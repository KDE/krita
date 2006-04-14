kde4_header()

set(kritathreadtest_PART_SRCS threadtest.cc )

kde4_automoc(${kritathreadtest_PART_SRCS})

kde4_add_plugin(kritathreadtest ${kritathreadtest_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritathreadtest )

target_link_libraries(kritathreadtest  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kritathreadtest )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritathreadtest.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritathreadtest.desktop
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	 -I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$(srcdir)/../../../../lib/kofficecore \
#	$(all_includes)
#
#kritathreadtest_la_SOURCES = threadtest.cc
#
#kde_module_LTLIBRARIES = kritathreadtest.la
#noinst_HEADERS = threadtest.h
#
#kritathreadtest_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritathreadtest_la_LIBADD =  ../../../libkritacommon.la
#
#kritathreadtest_la_METASOURCES = AUTO
