INCLUDES  = $(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

## The part
kde_module_LTLIBRARIES = libkritapart.la
libkritapart_la_SOURCES = dummy.cc

libkritapart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkritapart_la_LIBADD = core/libkiscore.la ui/libkisui.la $(LIB_KOFFICEUI) $(LIB_KOPAINTER) tools/libkistools.la

METASOURCES = AUTO

## The kdeinit loadable module
lib_LTLIBRARIES = krita.la
krita_la_SOURCES = main.cc
krita_la_LDFLAGS = $(all_libraries) -avoid-version -module
krita_la_LIBADD = $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

## The executable
bin_PROGRAMS = krita
krita_SOURCES = dummmmy.cc
krita_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krita_LDADD = krita.la

dummy.cc:
	echo > dummy.cc

dummmmy.cc:
	echo > dummmmy.cc

kdelnk_DATA = krita.desktop
kdelnkdir = $(kde_appsdir)/Office

#kdemime_DATA = x-krita.desktop
kdemimedir = $(kde_mimedir)/application

rcdir = $(kde_datadir)/krita
rc_DATA = krita.rc krita_readonly.rc

SUBDIRS = core ui tools . dtd plugins data pics 

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cc */*.cc core/kis_aboutdata.h -o $(podir)/krita.pot
