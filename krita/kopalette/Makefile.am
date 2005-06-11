# all_includes must remain last!
INCLUDES  = -I $(srcdir) \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)
	
#CXXFLAGS = -shared -fPIC

noinst_LTLIBRARIES = libkopalette.la

libkopalette_la_SOURCES = \
	koPalette.cc \
	koPaletteManager.cc \
	koToolBoxPalette.cc \
	koTabPalette.cc

noinst_HEADERS =  \
	koPalette.h \
	koTabPalette.h \
	koToolBoxPalette.h

libkopalette_la_METASOURCES = AUTO
libkopalette_la_LDFLAGS = $(all_libraries)
KDE_OPTIONS = nofinal
