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
	-I${srcdir}/capability_mediator \
	 $(all_includes)

#CXXFLAGS = -shared -fPIC

   
noinst_LTLIBRARIES = libkiscore.la

libkiscore_la_LIBADD = tiles/libkistile.la helper/libkishelper.la \
	builder/libkiscorebuilder.la \
	strategy/libkisstrategy.la \
	color_strategy/libkiscolor_strategy.la \
	capability_mediator/libkiscapabilitymediator.la

libkiscore_la_SOURCES = kis_paint_device.cc \
	kis_layer.cc \
	kis_channel.cc \
	kis_mask.cc \
	kis_image.cc \
	kis_doc.cc \
	kis_view.cc \
	kis_factory.cc \
	kis_pluginserver.cc \
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
	KRayonViewIface.cc \
	KRayonViewIface.skel \
	KIsDocIface.cc \
	KIsDocIface.skel \
	KIsImageIface.cc \
	KIsImageIface.skel

libkiscore_la_METASOURCES = AUTO
libkiscore_la_LDFLAGS = $(KDE_RPATH)
SUBDIRS = helper tiles builder strategy color_strategy capability_mediator
