INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../../ \
	-I$(srcdir)/../../../core/ \
	-I$(srcdir)/../../../core/color_strategy \
	-I$(srcdir)/../../../core/resources \
	-I$(srcdir)/../../../core/compositeop \
	-I$(srcdir)/../../../core/paintop \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../../core/tool \
	-I$(srcdir)/../../../core/tiles \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiskjsembedobjects.la

libkiskjsembedobjects_la_SOURCES = kis_paint_device_object.cc \
	kis_objects_factory.cc

# kritakjsembedobjects_la_LIBADD = -lkjs

noinst_HEADERS = 

libkiskjsembedobjects_la_METASOURCES = AUTO
