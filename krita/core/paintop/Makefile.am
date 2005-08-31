INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../sdk \
        -I$(srcdir)/../tiles \
        -I$(srcdir)/../tool \
        -I$(srcdir)/../filters \
        -I$(srcdir)/../color_strategy \
        -I$(srcdir)/../../paintops \
        -I$(srcdir)/../../ui \
        -I$(srcdir)/../resources \
        $(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkispaintop.la

libkispaintop_la_SOURCES =  \
	kis_paintop.cc \
	kis_paintop_registry.cc

 
libkispaintop_la_METASOURCES = AUTO

