

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kofficescan_PART_SRCS scan.cpp )

kde4_automoc(${kofficescan_PART_SRCS})

kde4_add_plugin(kofficescan ${kofficescan_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kofficescan )

target_link_libraries(kofficescan  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KIO_LIBS} kofficecore )

install(TARGETS kofficescan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scan-kpresenter.rc  DESTINATION ${DATA_INSTALL_DIR}/kpresenter/kpartplugins)
install( FILES  scan-krita.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/kpartplugins)
install( FILES  scan-kword.rc  DESTINATION ${DATA_INSTALL_DIR}/kword/kpartplugins)





#original Makefile.am contents follow:

#kwordrcdir = $(kde_datadir)/kword/kpartplugins
#kwordrc_DATA = scan-kword.rc
#
#kritarcdir = $(kde_datadir)/krita/kpartplugins
#kritarc_DATA = scan-krita.rc
#
#kpresenterrcdir = $(kde_datadir)/kpresenter/kpartplugins
#kpresenterrc_DATA = scan-kpresenter.rc
#
#EXTRA_DIST = $(kwordrc_DATA) $(kritarc_DATA) $(kpresenterrc_DATA)
#
#INCLUDES = $(KOFFICE_INCLUDES) $(all_includes)
#kde_module_LTLIBRARIES = kofficescan.la
#
#kofficescan_la_SOURCES = scan.cpp
#noinst_HEADERS = scan.h
#
#kofficescan_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kofficescan_la_LIBADD = $(LIB_KPARTS) $(LIB_KIO) $(LIB_KOFFICECORE)
#
#kofficescan_la_METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kscan_plugin.pot
