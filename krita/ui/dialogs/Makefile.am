INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tool \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)
noinst_LTLIBRARIES = libkisdialogs.la
libkisdialogs_la_SOURCES = dummy.cc \
	wdgmatrix.ui \
	wdghistogram.ui \
	kis_previewwidgetbase.ui \
	wdgselectionoptions.ui \
	kis_preview_dialog.ui \
	wdgnewimage.ui \
	wdgcolorsettings.ui \
        wdgapplyprofile.ui \
	wdgdockertab.ui 

noinst_HEADERS = wdgmatrix.h \
	wdghistogram.h \
	kis_previewwidgetbase.h \
	wdgnewimage.h \
	wdgcolorsettings.h \
        wdgapplyprofile.h \
	wdgdockertab.h

METASOURCES = AUTO
