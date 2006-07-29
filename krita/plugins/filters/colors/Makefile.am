kritarcdir = $(kde_datadir)/krita/kritaplugins

kde_services_DATA = kritaextensioncolorsfilters.desktop

INCLUDES = -I$(srcdir)/../../../sdk \
        -I$(srcdir)/../../../core \
        -I$(srcdir)/../../../kritacolor/ \
        -I$(srcdir)/../../../ui \
        $(KOFFICE_INCLUDES) \
        $(all_includes)

kritaextensioncolorsfilters_la_SOURCES = colors.cc kis_minmax_filters.cc kis_color_to_alpha.cc wdgcolortoalphabase.ui kis_wdg_color_to_alpha.cc

kde_module_LTLIBRARIES = kritaextensioncolorsfilters.la
noinst_HEADERS = colors.h

kritaextensioncolorsfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaextensioncolorsfilters_la_LIBADD = ../../../libkritacommon.la

METASOURCES = AUTO
