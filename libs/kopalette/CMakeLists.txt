


include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kopalette_LIB_SRCS 
   kopalette.cc 
   kopalettemanager.cc 
   kotoolboxpalette.cc 
   kotabpalette.cc
   KoPaletteTabBar.cpp
   KoPaletteTabWidget.cpp
   KoIconTabPalette.cpp)

kde4_automoc(${kopalette_LIB_SRCS})

kde4_add_library(kopalette SHARED ${kopalette_LIB_SRCS})

target_link_libraries(kopalette  ${KDE4_KDECORE_LIBS} kofficecore )

set_target_properties(kopalette PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kopalette )


########### install files ###############

install_files( /include FILES  kopalettemanager.h  )





#original Makefile.am contents follow:

## all_includes must remain last!
#INCLUDES  = -I$(srcdir) \
#	$(KOFFICECORE_INCLUDES) \
#    $(all_includes)
#
#lib_LTLIBRARIES = libkopalette.la
#
#libkopalette_la_SOURCES = \
#	kopalette.cc \
#	kopalettemanager.cc \
#	kotoolboxpalette.cc \
#	kotabpalette.cc
#
#libkopalette_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
#libkopalette_la_LIBADD  = $(LIB_KOFFICECORE)
#
#include_HEADERS =  \
#    kopalettemanager.h 
#
#noinst_HEADERS = \
#	kopalette.h \
#	kotabpalette.h \
#	kotoolboxpalette.h
#
#METASOURCES = AUTO
