# all_includes must remain last!
INCLUDES  = -I$(srcdir)/builder \
	-I$(srcdir)/resources \
	-I$(srcdir)/visitors \
	-I$(srcdir)/paintop \
	-I$(srcdir)/tiles \
	-I$(srcdir)/../ui \
	-I$(srcdir)/tool \
	-I../ui \
	-I$(srcdir)/../ui/dialogs \
	-I../ui/dialogs \
	-I$(srcdir)/../ui/widgets \
	-I../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	-I$(srcdir)/strategy \
	-I$(srcdir)/color_strategy \
	$(all_includes)

#CXXFLAGS = -shared -fPIC

noinst_LTLIBRARIES = libkritacore.la

libkritacore_la_LIBADD = tiles/libkistile.la \
	builder/libkiscorebuilder.la \
	strategy/libkisstrategy.la \
	color_strategy/libkiscolor_strategy.la \
	resources/libkisresources.la \
	paintop/libkispaintop.la \
	tool/libkistool.la \
	$(LIB_KOPAINTER)

libkritacore_la_SOURCES = kis_paint_device.cc \
	kis_layer.cc \
	kis_image.cc \
	kis_doc.cc \
	kis_view.cc \
	kis_factory.cc \
	kis_nameserver.cc \
	kis_painter.cc \
	kis_fill_painter.cc \
	kis_gradient_painter.cc \
	kis_convolution_painter.cc \
	kis_cursor.cc \
	kis_selection.cc \
	kis_command.cc \
	kis_background.cc \
	kis_global.cc \
	kis_config.cc \
	kis_guide.cc \
	kis_canvas_subject.cc \
	kis_canvas_controller.cc \
	kis_canvas_observer.cc \
	kis_tile_command.cc \
	kis_vec.cc \
	kis_rect.cc \
	kis_iterators.cc \
	kis_iterators_quantum.cc \
	kis_iterators_pixel.cc \
	kis_iterators_infinite.cc \
	kis_histogram.cc \
	KRayonViewIface.cc \
	KRayonViewIface.skel \
	KIsDocIface.cc \
	KIsDocIface.skel \
	KIsImageIface.cc \
	KIsImageIface.skel \
	kis_filter.cc \
	kis_plugin_registry.cc \
	kis_filter_registry.cc \
	kis_selection_manager.cc \
	kis_scale_visitor.cc \
        kis_rotate_visitor.cc \
        kis_cms.cc

noinst_HEADERS = kis_paint_device.h \
	kis_layer.h \
	kis_image.h \
	kis_doc.h \
	kis_view.h \
	kis_factory.h \
	kis_nameserver.h \
	kis_painter.h \
	kis_cursor.h \
	kis_selection.h \
	kis_command.h \
	kis_background.h \
	kis_global.h \
	kis_config.h \
	kis_guide.h \
	kis_canvas_subject.h \
	kis_canvas_controller.h \
	kis_canvas_observer.h \
	kis_tile_command.h \
	kis_vec.h \
	kis_rect.h \
	kis_iterators.h \
	kis_iterators_quantum.h \
	kis_iterators_pixel.h \
	kis_iterators_infinite.h \
	kis_histogram.h \
	kis_plugin_registry.h \
	KRayonViewIface.h \
	KIsDocIface.h \
	KIsImageIface.h \
	kis_matrix.h \
	kis_filter.h \
	kis_filter_registry.h \
	kis_selection_manager.h \
        kis_scale_visitor.h \
        kis_rotate_visitor.h \
        kis_cms.h

libkritacore_la_METASOURCES = AUTO
libkritacore_la_LDFLAGS = $(all_libraries)
SUBDIRS = tiles builder strategy color_strategy paintop resources tool
KDE_OPTIONS = nofinal
