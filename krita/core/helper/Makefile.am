
INCLUDES  = -I$(srcdir)/../../ui -I$(srcdir)/.. -I$(srcdir)/../../tools $(KOFFICE_INCLUDES) -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkishelper.la

libkishelper_la_SOURCES = kisscopedlock.cpp
libkishelper_la_METASOURCES = AUTO
libkishelper_la_LDFLAGS = $(KDE_RPATH) $(LIBMAGICK_RPATH)
libkishelper_la_LIBADD = $(LIBMAGICK_LIBS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

