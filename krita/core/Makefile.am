# all_includes must remain last!
INCLUDES  = -I$(srcdir)/builder \
	 -I$(srcdir)/visitors \
	 -I$(srcdir)/helper/ \
	-I$(srcdir)/tiles \
	-I$(srcdir)/../ui -I../ui \
	-I$(srcdir)/../tools \
	$(KOFFICE_INCLUDES) \
	 -I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	-I$(srcdir)/strategy \
	-I$(srcdir)/color_strategy \
	 $(all_includes)

#CXXFLAGS = -shared -fPIC

noinst_LTLIBRARIES = libkritacore.la

libkritacore_la_LIBADD = tiles/libkistile.la \
	helper/libkishelper.la \
	builder/libkiscorebuilder.la \
	strategy/libkisstrategy.la \
	color_strategy/libkiscolor_strategy.la \
	$(LIB_KOPAINTER)

libkritacore_la_SOURCES = kis_paint_device.cc \
	kis_layer.cc \
	kis_channel.cc \
	kis_mask.cc \
	kis_image.cc \
	kis_doc.cc \
	kis_view.cc \
	kis_factory.cc \
	kis_resourceserver.cc \
	kis_brush.cc \
	kis_pattern.cc \
	kis_nameserver.cc \
	kis_painter.cc \
	kis_tool_factory.cc \
	kis_cursor.cc \
	kis_floatingselection.cc \
	kis_selection.cc \
	kis_command.cc \
	kis_background.cc \
	kis_global.cc \
	kis_config.cc \
	kis_tool.cc \
	kis_resource.cc \
	kis_resource_mediator.cc \
	kis_guide.cc \
	kis_memento_originator.cc \
	kis_canvas_subject.cc \
	kis_canvas_controller.cc \
	kis_canvas_observer.cc \
	kis_tool_controller.cc \
	kis_tile_command.cc \
	kis_alpha_mask.cc \
	kis_imagepipe_brush.cc \
	kis_gradient.cc \
	kis_vec.cc \
	kis_rect.cc \
	kis_iterators.cc \
	kis_iterators_quantum.cc \
	kis_iterators_pixel.cc \
	kis_histogram.cc \
	kis_plugin_registry.cc \
	KRayonViewIface.cc \
	KRayonViewIface.skel \
	KIsDocIface.cc \
	KIsDocIface.skel \
	KIsImageIface.cc \
	KIsImageIface.skel

libkritacore_la_METASOURCES = AUTO
libkritacore_la_LDFLAGS = $(all_libraries)
SUBDIRS = helper tiles builder strategy color_strategy 
