INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../color_strategy \
	-I$(srcdir)/../compositeop \
	-I$(srcdir)/../visitors/ \
	-I$(srcdir)/../tiles/ \
	-I$(srcdir)/../../ui/ \
	-I../../ui/ \
	$(KOFFICE_INCLUDES) -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkiscorebuilder.la
libkiscorebuilder_la_SOURCES = kis_image_magick_converter.cc kis_builder_subject.cc kis_builder_monitor.cc

libkiscorebuilder_la_METASOURCES = AUTO
libkiscorebuilder_la_LDFLAGS = $(LIBMAGICK_LDFLAGS) $(KDE_RPATH) $(LIBMAGICK_RPATH)
libkiscorebuilder_la_LIBADD = $(LIBMAGICK_LIBS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

