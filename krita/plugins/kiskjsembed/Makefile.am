kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritakjsembed.rc

SUBDIRS = functions objects

EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../ \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../libkritacolor/ \
	-I$(srcdir)/../../libkritacolor/color_strategy \
	-I$(srcdir)/../../ui \
	 $(all_includes)

kde_module_LTLIBRARIES = kritakjsembed.la

kritakjsembed_la_SOURCES = kis_kjsembed.cc \
	kis_script.cc \
	kis_function_base.cc \
	kis_object_base.cc

noinst_HEADERS = kis_kjsembed.h kis_script.h kis_function_base.h

kritakjsembed_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritakjsembed_la_LIBADD =  ../../libkritacommon.la -lkjsembed  ./functions/libkiskjsembedfunctions.la ./objects/libkiskjsembedobjects.la

kritakjsembed_la_METASOURCES = AUTO
