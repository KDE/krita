kde4_header()

add_subdirectory( kritascripting )
add_subdirectory( kritacore )
add_subdirectory( samples )

include_directories( ${CMAKE_SOURCE_DIR}/krita/sdk ${CMAKE_SOURCE_DIR}/krita/core ${CMAKE_SOURCE_DIR}/krita/kritacolor/ ${CMAKE_SOURCE_DIR}/krita/ui ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kritascripting_PART_SRCS scripting.cc)

kde4_automoc(${kritascripting_PART_SRCS})

kde4_add_plugin(kritascripting ${kritascripting_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritascripting )

target_link_libraries(kritascripting  ${KDE4_KDECORE_LIBS} kritacommon krossmain kritascripting )

install_targets(${LIB_INSTALL_DIR}/kde4 kritascripting )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kritaplugins FILES  scripting.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritascripting.desktop )

kde4_footer()



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
