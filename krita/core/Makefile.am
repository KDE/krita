INCLUDES  = -I$(srcdir)/helper/ -I$(srcdir)/tiles/ -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkiscore.la

libkiscore_la_SOURCES = kis_paint_device.cc kis_layer.cc kis_channel.cc kis_mask.cc kis_image.cc kis_doc.cc\
	kis_view.cc kis_tool.cc kis_factory.cc kis_pluginserver.cc kis_resourceserver.cc kis_timer.cc \
	kis_brush.cc kis_krayon.cc kis_pattern.cc kis_image_builder.cc kis_util.cc kis_nameserver.cc

libkiscore_la_METASOURCES = AUTO
libkiscore_la_LDFLAGS = $(KDE_RPATH) $(LIBMAGICK_RPATH)
libkiscore_la_LIBADD = $(LIBMAGICK_LIBS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)
SUBDIRS = helper tiles 

