
INCLUDES  = -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS)  $(all_includes)

noinst_LTLIBRARIES = libkiscore.la

libkiscore_la_SOURCES = kis_pixel_packet.cc \
	kis_view.cc kis_channel.cc \
	 kis_layer.cc kis_image.cc \
    kis_pluginserver.cc kis_resourceserver.cc \
	kis_factory.cc kis_config.cc kis_undo.cc \
    kis_doc.cc \
    kis_krayon.cc kis_brush.cc kis_pattern.cc kis_gradient.cc \
	kis_util.cc kis_tool.cc kis_vec.cc \
	kis_cursor.cc kis_timer.cc \
    kis_selection.cc kis_framebuffer.cc kis_painter.cc kis_tool_factory.cc \
	KRayonViewIface.skel KRayonViewIface.cc kis_tile.cc kis_tiles.cc \
	kis_paint_device.cc KIsImageIface.skel KIsImageIface.cc KIsDocIface.skel \
	KIsDocIface.cc kis_image_cmd.cc kis_mask.cc kis_color_utils.cc \
	kis_magick.cc

libkiscore_la_METASOURCES = AUTO
libkiscore_la_LDFLAGS = $(KDE_RPATH) $(LIBMAGICK_RPATH)
libkiscore_la_LIBADD = $(LIBMAGICK_LIBS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

