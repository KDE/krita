INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../sdk \
	$(KOFFICE_INCLUDES) \
	 $(all_includes)

noinst_LTLIBRARIES = libkritacolorspaces.la

libkritacolorspaces_la_SOURCES = \
	kis_alpha_colorspace.cc \
	kis_lab_colorspace.cc 

noinst_HEADERS = \
    kis_alpha_colorspace.h  \
    kis_lab_colorspace.h

libkritacolorspaces_la_LIBADD = $(OPENEXR_LIBS)

libkritacolorspaces_la_METASOURCES = AUTO


