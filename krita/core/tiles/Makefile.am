
INCLUDES  = -I$(srcdir)/../helper -I$(srcdir)/../../ui -I$(srcdir)/.. -I$(srcdir)/../../tools $(KOFFICE_INCLUDES) -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkistile.la

libkistile_la_SOURCES = kistile.cpp kistilemgr.cpp kistilemediator.cpp kispixeldata.cpp
libkistile_la_METASOURCES = AUTO
libkistile_la_LDFLAGS = $(KDE_RPATH) $(LIBMAGICK_RPATH)
libkistile_la_LIBADD = $(LIBMAGICK_LIBS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

