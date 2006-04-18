kde4_header()

set(kritahistorydocker_PART_SRCS historydocker.cc )

kde4_automoc(${kritahistorydocker_PART_SRCS})

kde4_add_plugin(kritahistorydocker ${kritahistorydocker_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistorydocker )

target_link_libraries(kritahistorydocker  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${PLUGIN_INSTALL_DIR} kritahistorydocker )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##kde_services_DATA = kritahistorydocker.desktop
#
#INCLUDES  = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritahistorydocker_la_SOURCES = historydocker.cc 
#
#kde_module_LTLIBRARIES = kritahistorydocker.la
#noinst_HEADERS = historydocker.h 
#
#kritahistorydocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritahistorydocker_la_LIBADD =  ../../../libkritacommon.la
#
#kritahistorydocker_la_METASOURCES = AUTO
