kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritacolorifyfilter.desktop

INCLUDES = -I$(srcdir)/../../../sdk \
        -I$(srcdir)/../../../core \
        -I$(srcdir)/../../../kritacolor/ \
        -I$(srcdir)/../../../ui \
        $(KOFFICE_INCLUDES) \
        $(all_includes)



kde_module_LTLIBRARIES = kritacolorify.la


kritacolorify_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritacolorify_la_LIBADD =  ../../../libkritacommon.la

METASOURCES = AUTO
kritacolorify_la_SOURCES = Colorify.cpp KisWdgColorify.cpp\
	WdgColorifyBase.ui
