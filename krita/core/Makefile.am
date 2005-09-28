# all_includes must remain last!
INCLUDES  = \
	-I$(srcdir)/../sdk \
	-I$(srcdir)/resources \
	-I$(srcdir)/visitors \
	-I$(srcdir)/tool \
	-I$(srcdir)/paintop \
	-I$(srcdir)/tiles \
	-I$(srcdir)/../ui \
	-I$(srcdir)/../kopalette \
	-I../ui \
	-I../kopalette \
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
	strategy/libkisstrategy.la \
	color_strategy/libkiscolor_strategy.la \
	resources/libkisresources.la \
	paintop/libkispaintop.la \
	tool/libkistool.la \
	$(LIB_KOPAINTER)

libkritacore_la_SOURCES = kis_color.cc kis_paint_device_impl.cc kis_layer.cc \
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
    kis_boundary.cc kis_histogram_producer.cc


noinst_HEADERS = kis_paint_device_impl.h kis_layer.h kis_image.h kis_doc.h \
	kis_view.h kis_factory.h kis_nameserver.h kis_cursor.h kis_selection.h \
	kis_command.h kis_background.h kis_config.h  \
	kis_transaction.h kis_vec.h kis_rect.h \
	kis_iteratorpixeltrait.h kis_iterators_pixel.h kis_view_iface.h kis_doc_iface.h kis_image_iface.h \
	kis_matrix.h kis_selection_manager.h kis_scale_visitor.h kis_rotate_visitor.h \
	kis_filter.h kis_filter_registry.h kis_part_layer.h \
	kis_filter_manager.h kis_tool_manager.h kis_histogram.h \
	kis_paint_device_iface.h kis_boundary.h kis_histogram_producer.h


libkritacore_la_METASOURCES = AUTO
libkritacore_la_LDFLAGS = $(all_libraries)
KDE_OPTIONS = nofinal

if include_kunittest_tests
TESTSDIR = tests
endif

SUBDIRS = tiles strategy color_strategy paintop resources tool . $(TESTSDIR)

