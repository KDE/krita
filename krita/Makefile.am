INCLUDES  = $(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

## The part
lib_LTLIBRARIES = 
kde_module_LTLIBRARIES = libkritapart.la
libkritapart_la_SOURCES = dummy.cc

libkritapart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkritapart_la_LIBADD = core/libkiscore.la ui/libkisui.la $(LIB_KOFFICEUI) $(LIB_KOPAINTER) tools/libkistools.la

METASOURCES = AUTO

## The kdeinit loadable module and executable
kdeinit_LTLIBRARIES = krita.la
bin_PROGRAMS =
krita_la_SOURCES = main.cc
krita_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_la_LIBADD = $(LIB_KOFFICECORE)

kdelnk_DATA = krita.desktop
kdelnkdir = $(kde_appsdir)/Office

#kdemime_DATA = x-krita.desktop
kdemimedir = $(kde_mimedir)/application

rcdir = $(kde_datadir)/krita
rc_DATA = krita.rc krita_readonly.rc

SUBDIRS = core ui tools . dtd plugins data pics 

dummy.cc:
	echo > dummy.cc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cc */*.cc core/kis_aboutdata.h -o $(podir)/krita.pot
