kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = modify_selection.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritamodifyselection.desktop


INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritamodifyselection_la_SOURCES = wdg_grow_selection.ui wdg_shrink_selection.ui wdg_border_selection.ui \
dlg_grow_selection.cc dlg_shrink_selection.cc dlg_border_selection.cc modify_selection.cc

noinst_HEADERS = wdg_grow_selection.h wdg_shrink_selection.h wdg_border_selection.h dlg_grow_selection.h \
dlg_shrink_selection.h dlg_border_selection.h modify_selection.h

kde_module_LTLIBRARIES = kritamodifyselection.la

kritamodifyselection_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritamodifyselection_la_LIBADD =  ../../../libkritacommon.la 

kritamodifyselection_la_METASOURCES = AUTO
