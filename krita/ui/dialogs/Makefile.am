INCLUDES  = -I$(srcdir)/.. \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tool \
	-I.. \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)
noinst_LTLIBRARIES = libkisdialogs.la
libkisdialogs_la_SOURCES = dummy.cc \
	wdgmatrix.ui \
	new_image.ui \
	wdghistogram.ui \
	kis_previewwidgetbase.ui \
	wdgselectionoptions.ui \
	kis_preview_dialog.ui 

noinst_HEADERS = wdgmatrix.h \
	wdghistogram.h \
	new_image.h \
	kis_previewwidgetbase.h

METASOURCES = AUTO
