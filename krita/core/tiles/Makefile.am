INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../color_strategy \
	-I$(srcdir)/../compositeop \
	-I$(srcdir)/../helper \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkistile.la

libkistile_la_SOURCES = kistile.cc kistilemgr.cc kistilemediator.cc kispixeldata.cc
libkistile_la_METASOURCES = AUTO

