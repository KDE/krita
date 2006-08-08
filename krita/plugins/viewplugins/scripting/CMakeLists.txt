

include_directories(
	BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/kritascripting
	${KROSS_INCLUDES}
	)

add_subdirectory( kritascripting )
#add_subdirectory( kritacore )
add_subdirectory( samples )

########### next target ###############

set(kritascripting_PART_SRCS scripting.cc)

kde4_automoc(${kritascripting_PART_SRCS})

kde4_add_plugin(kritascripting_plug ${kritascripting_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritascripting_plug )
set_target_properties(kritascripting_plug PROPERTIES OUTPUT_NAME kritascripting)

target_link_libraries(kritascripting_plug kritaui krossmain kritascripting)

install(TARGETS kritascripting_plug  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scripting.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritascripting.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#SUBDIRS = kritascripting kritacore samples
#
#kritarcdir = $(kde_datadir)/kritaplugins
#kritarc_DATA = scripting.rc
#
#kde_services_DATA = kritascripting.desktop
#
#
#INCLUDES  =  -I$(top_srcdir)/krita/sdk \
#	-I$(top_srcdir)/krita/core \
#	-I$(top_srcdir)/krita/kritacolor/ \
#	-I$(top_srcdir)/krita/ui \
#        -I$(top_builddir)/lib/kross/main \
#	$(KROSS_INCLUDES) \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritascripting_la_SOURCES = scripting.cc
#
#kde_module_LTLIBRARIES = kritascripting.la
#noinst_HEADERS = scripting.h
#
#kritascripting_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritascripting_la_LIBADD =  $(top_builddir)/krita/libkritacommon.la $(top_builddir)/lib/kross/main/libkrossmain.la ./kritascripting/libkritascripting.la
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#kritascripting_la_METASOURCES = AUTO
