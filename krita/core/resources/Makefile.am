INCLUDES  = -I$(srcdir)/.. \
        -I$(srcdir)/../tiles \
        -I$(srcdir)/../../ui \
        -I$(srcdir)/../color_strategy \
        $(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkisresources.la

libkisresources_la_SOURCES =  \
	kis_alpha_mask.cc \
	kis_brush.cc \
	kis_gradient.cc \
	kis_imagepipe_brush.cc \
	kis_pattern.cc \
	kis_resource.cc \
	kis_resource_mediator.cc \
	kis_resourceserver.cc

noinst_HEADERS = \
	kis_alpha_mask.h \
	kis_brush.h \
	kis_gradient.h \
	kis_imagepipe_brush.h \
	kis_pattern.h \
	kis_resource.h \
	kis_resource_mediator.h \
	kis_resourceserver.h
 
libkisresources_la_METASOURCES = AUTO

