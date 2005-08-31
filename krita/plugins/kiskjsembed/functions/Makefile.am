INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../ \
	-I$(srcdir)/../../../ \
	-I$(srcdir)/../../../core/ \
	-I$(srcdir)/../../../sdk/ \
	-I$(srcdir)/../../../core/color_strategy \
	-I$(srcdir)/../../../core/resources \
	-I$(srcdir)/../../../core/compositeop \
	-I$(srcdir)/../../../core/paintop \
	-I$(srcdir)/../../../core/tool \
	-I$(srcdir)/../../../core/tiles \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiskjsembedfunctions.la

libkiskjsembedfunctions_la_SOURCES = kis_mainwindow_functions.cc \
	kis_functions_factory.cc

noinst_HEADERS = kis_mainwindow_functions.h

libkiskjsembedfunctions_la_METASOURCES = AUTO
