

########### next target ###############

set(kritascripting_LIB_SRCS kis_script_progress.cpp kis_script_monitor.cpp )

kde4_automoc(${kritascripting_LIB_SRCS})

kde4_add_library(kritascripting SHARED ${kritascripting_LIB_SRCS})

target_link_libraries(kritascripting kritaui krossmain)

set_target_properties(kritascripting PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kritascripting )


########### install files ###############






#original Makefile.am contents follow:

#INCLUDES  =  -I$(top_srcdir)/krita/sdk \
#	-I$(top_srcdir)/krita/core \
#	-I$(top_srcdir)/krita/kritacolor/ \
#	-I$(top_srcdir)/krita/ui \
#	$(KROSS_INCLUDES) \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#lib_LTLIBRARIES = libkritascripting.la
#libkritascripting_la_SOURCES = kis_script_progress.cpp kis_script_monitor.cpp
#noinst_HEADERS = kis_script_progress.h
#
#libkritascripting_la_LDFLAGS = -no-undefined $(all_libraries)
#libkritascripting_la_LIBADD =  $(top_builddir)/krita/libkritacommon.la $(top_builddir)/lib/kross/main/libkrossmain.la
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#libkritascripting_la_METASOURCES = AUTO
