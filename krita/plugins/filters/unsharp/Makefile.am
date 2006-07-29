kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritaunsharpfilter.desktop

INCLUDES =  -I$(srcdir)/../../../sdk \
        -I$(srcdir)/../../../core \
        -I$(srcdir)/../../../kritacolor \
        -I$(srcdir)/../../../ui \
        $(KOFFICE_INCLUDES) \
        $(all_includes)

kritaunsharpfilter_la_SOURCES = wdgunsharp.ui kis_wdg_unsharp.cc unsharp.cc kis_unsharp_filter.cc 

kde_module_LTLIBRARIES = kritaunsharpfilter.la

kritaunsharpfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaunsharpfilter_la_LIBADD = ../../../libkritacommon.la

METASOURCES = AUTO
