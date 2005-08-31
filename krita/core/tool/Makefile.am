INCLUDES  = -I$(srcdir)/.. \
        -I$(srcdir)/../../ui \
	-I$(srcdir)/../../sdk \
	-I../../ui/ \
        -I$(srcdir)/../tiles \
        -I$(srcdir)/../paintop \
        -I$(srcdir)/../resources \
        -I$(srcdir)/../color_strategy \
        $(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkistool.la

libkistool_la_SOURCES =  \
	kis_tool.cc \
	kis_tool_registry.cc \
	kis_tool_paint.cc \
	kis_tool_freehand.cc \
	kis_tool_non_paint.cc \
	kis_tool_shape.cc \
	kis_tool_dummy.cc

libkistool_la_METASOURCES = AUTO

