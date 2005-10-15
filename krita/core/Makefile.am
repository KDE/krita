# all_includes must remain last!
INCLUDES  = \
	-I$(srcdir)/../sdk \
	-I$(srcdir)/tiles \
	-I$(srcdir)/../libkritacolor \
	-I$(srcdir)/../libkritacolor/color_strategy \
	-I$(srcdir)/../ui \
	-I../ui \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
    $(OPENEXR_CFLAGS) \
	$(all_includes)

#CXXFLAGS = -shared -fPIC

noinst_LTLIBRARIES = libkritacore.la

libkritacore_la_LIBADD = tiles/libkistile.la \
	$(LIB_KOPAINTER) $(OPENEXR_LIBS)

libkritacore_la_SOURCES = kis_paint_device_impl.cc kis_layer.cc \
	kis_image.cc kis_doc.cc kis_view.cc kis_factory.cc kis_nameserver.cc \
	kis_painter.cc kis_fill_painter.cc kis_gradient_painter.cc \
	kis_convolution_painter.cc kis_cursor.cc kis_selection.cc kis_command.cc kis_background.cc \
	kis_config.cc kis_transaction.cc \
	kis_selected_transaction.cc kis_vec.cc kis_rect.cc kis_iterators_pixel.cc kis_view_iface.cc \
	kis_view_iface.skel kis_doc_iface.cc kis_doc_iface.skel kis_image_iface.cc kis_image_iface.skel \
	kis_selection_manager.cc kis_scale_visitor.cc kis_transform_visitor.cc kis_rotate_visitor.cc \
	kis_clipboard.cc kis_filter.cc kis_filter_registry.cc kis_filter_strategy.cc \
	kis_part_layer.cc kis_filter_manager.cc kis_tool_manager.cc \
	kis_paint_device_iface.cc kis_paint_device_iface.skel kis_histogram.cc \
    kis_boundary.cc kis_paintop.cc kis_paintop_registry.cc \
    kis_strategy_move.cc \
    kis_tool.cc \
    kis_tool_dummy.cc \
    kis_tool_freehand.cc \
    kis_tool_non_paint.cc \
    kis_tool_paint.cc \
    kis_tool_registry.cc \
    kis_tool_shape.cc \
    kis_alpha_mask.cc \
    kis_autobrush_resource.cc \
    kis_autogradient_resource.cc \
    kis_brush.cc \
    kis_gradient.cc \
    kis_imagepipe_brush.cc \
    kis_palette.cc \
    kis_pattern.cc \
    kis_resource.cc \
    kis_resourceserver.cc \
    kis_iterator.cc kis_meta_registry.cc


noinst_HEADERS = kis_paint_device_impl.h kis_layer.h kis_image.h kis_doc.h \
	kis_view.h kis_factory.h kis_nameserver.h kis_cursor.h kis_selection.h \
	kis_command.h kis_background.h kis_config.h  \
	kis_transaction.h kis_vec.h kis_rect.h \
	kis_iteratorpixeltrait.h kis_iterators_pixel.h kis_view_iface.h kis_doc_iface.h kis_image_iface.h \
	kis_selection_manager.h kis_scale_visitor.h kis_rotate_visitor.h \
	kis_filter.h kis_filter_registry.h kis_part_layer.h \
	kis_filter_manager.h kis_tool_manager.h kis_histogram.h \
	kis_paint_device_iface.h kis_boundary.h kis_paintop_registry.h kis_paintop.h \
    kis_strategy_move.h \
    kis_tool_shape.h \
    kis_tool_registry.h \
    kis_tool_paint.h \
    kis_tool_non_paint.h \
    kis_tool_freehand.h \
    kis_tool_factory.h \
    kis_tool_dummy.h \
    kis_tool_controller.h \
    kis_tool.h \
    kis_resourceserver.h \
    kis_resource.h \
    kis_pattern.h \
    kis_palette.h \
    kis_imagepipe_brush.h \
    kis_gradient.h \
    kis_brush.h \
    kis_autogradient_resource.h \
    kis_autobrush_resource.h \
    kis_alpha_mask.h kis_datamanager.h kis_iterator.h


libkritacore_la_METASOURCES = AUTO
libkritacore_la_LDFLAGS = $(all_libraries)
KDE_OPTIONS = nofinal

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = tiles . $(TESTSDIR)

