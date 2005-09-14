INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../tiles \
	-I$(srcdir)/../resources \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(OPENEXR_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkiscolor_strategy.la

if have_openexr
OPENEXR_SOURCES=kis_f16half_base_colorspace.cc
endif

libkiscolor_strategy_la_SOURCES = kis_abstract_colorspace.cc \
	kis_colorspace_registry.cc \
	kis_alpha_colorspace.cc \
	kis_color_conversions.cc \
	kis_xyz_colorspace.cc \
    kis_u8_base_colorspace.cc \
    kis_u16_base_colorspace.cc \
    kis_f32_base_colorspace.cc \
    $(OPENEXR_SOURCES) \
    kis_basic_histogram_producers.cc
    #kis_abstract_colorspace_iface.cc

libkiscolor_strategy_la_LIBADD = $(OPENEXR_LIBS)

libkiscolor_strategy_la_METASOURCES = AUTO

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = . $(TESTSDIR)

