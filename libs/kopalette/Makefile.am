# all_includes must remain last!
INCLUDES  = -I$(srcdir) \
	$(KOFFICECORE_INCLUDES) \
    $(all_includes)

lib_LTLIBRARIES = libkopalette.la

libkopalette_la_SOURCES = \
	kopalette.cc \
	kopalettemanager.cc \
	kotoolboxpalette.cc \
	kotabpalette.cc

libkopalette_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
libkopalette_la_LIBADD  = $(LIB_KOFFICECORE)

include_HEADERS =  \
    kopalettemanager.h 

noinst_HEADERS = \
	kopalette.h \
	kotabpalette.h \
	kotoolboxpalette.h

METASOURCES = AUTO
