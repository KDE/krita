INCLUDES  = -I$(srcdir)/../ -I$(srcdir)/../color_strategy/ $(KOFFICE_INCLUDES)  -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiscapabilitymediator.la
libkiscapabilitymediator_la_LIBADD = ./paintops/libkispaintops.la ./compositeops/libkiscompositeops.la 
#./imagetypes/libkisimagetypes.la
libkiscapabilitymediator_la_SOURCES = kis_capability_mediator.cc \
	kis_abstract_capability.cc \
	kis_composite_op.cc \
	kis_paint_op.cc \
	kis_image_type.cc

libkiscapabilitymediator_la_METASOURCES = AUTO

SUBDIRS = paintops compositeops 
#imagetypes