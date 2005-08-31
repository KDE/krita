# all_includes must remain last!
INCLUDES  = -I$(srcdir) \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

#CXXFLAGS = -shared -fPIC

noinst_LTLIBRARIES = libkopalette.la

libkopalette_la_SOURCES = \
	kopalette.cc \
	kopalettemanager.cc \
	kotoolboxpalette.cc \
	kotabpalette.cc

noinst_HEADERS =  \
	kopalette.h \
	kotabpalette.h \
	kotoolboxpalette.h

libkopalette_la_METASOURCES = AUTO
libkopalette_la_LDFLAGS = $(all_libraries)
KDE_OPTIONS = nofinal
