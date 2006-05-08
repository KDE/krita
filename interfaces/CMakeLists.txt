kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kchart/kdchart ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kochart_LIB_SRCS koChart.cc )

kde4_automoc(${kochart_LIB_SRCS})

kde4_add_library(kochart SHARED ${kochart_LIB_SRCS})

target_link_libraries(kochart  ${KDE4_KDECORE_LIBS} kofficecore )

set_target_properties(kochart PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kochart )


########### install files ###############

install_files( /include FILES  koChart.h )



kde4_footer()



#original Makefile.am contents follow:

#INCLUDES  = $(KOFFICE_INCLUDES) -I$(top_srcdir)/kchart/kdchart -I$(interfacedir) $(all_includes)
#
#lib_LTLIBRARIES = libkochart.la
#
#libkochart_la_SOURCES = koChart.cc
#libkochart_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libkochart_la_LIBADD = $(LIB_KOFFICECORE)
#METASOURCES = AUTO
#
#include_HEADERS = koChart.h
#
#include $(top_srcdir)/admin/Doxyfile.am
