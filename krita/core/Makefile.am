# all_includes must remain last!
INCLUDES  = -I$(srcdir)/builder/ -I$(srcdir)/visitors/ -I$(srcdir)/helper/ -I$(srcdir)/tiles/ -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) \
	 -I$(interfacedir) $(KOPAINTER_INCLUDES) -I$(srcdir)/strategy/ $(all_includes)

noinst_LTLIBRARIES = libkiscore.la

libkiscore_la_LIBADD = tiles/libkistile.la helper/libkishelper.la builder/libkiscorebuilder.la strategy/libkisstrategy.la
libkiscore_la_SOURCES = kis_paint_device.cc kis_layer.cc kis_channel.cc kis_mask.cc kis_image.cc kis_doc.cc\
	kis_view.cc kis_factory.cc kis_pluginserver.cc kis_resourceserver.cc \
	kis_brush.cc kis_krayon.cc kis_pattern.cc kis_util.cc kis_nameserver.cc \
	kis_painter.cc kis_tool_memento.cc kis_tool_factory.cc kis_cursor.cc kis_selection.cc kis_command.cc \
	kis_background.cc kis_global.cc kis_config.cc kis_tool.cc 

libkiscore_la_METASOURCES = AUTO
libkiscore_la_LDFLAGS = $(KDE_RPATH)
SUBDIRS = helper tiles builder strategy

