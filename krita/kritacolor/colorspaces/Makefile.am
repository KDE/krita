INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../sdk \
	 $(all_includes)

noinst_LTLIBRARIES = libkiscolorspaces.la

libkiscolorspaces_la_SOURCES = \
	kis_alpha_colorspace.cc \
	kis_xyz_colorspace.cc 

noinst_HEADERS = \
    kis_alpha_colorspace.h  \
    kis_xyz_colorspace.h

libkiscolorspaces_la_LIBADD = $(OPENEXR_LIBS)

libkiscolorspaces_la_METASOURCES = AUTO


