
INCLUDES  = $(KOFFICE_INCLUDES) -I$(interfacedir) $(all_includes)

## The part
kde_module_LTLIBRARIES = libkritapart.la
libkritapart_la_SOURCES = krita_part.cpp \
	krita_view.cpp krita_factory.cpp
libkritapart_la_LDFLAGS = $(KDE_PLUGIN)
libkritapart_la_LIBADD = -lkofficeui

## The kdeinit loadable module
lib_LTLIBRARIES = krita.la
krita_la_SOURCES = main.cpp
krita_la_LDFLAGS = $(all_libraries) -module -avoid-version
krita_la_LIBADD = -lkofficecore

## The executable
bin_PROGRAMS = krita
krita_SOURCES = dummy.cpp
krita_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krita_LDADD = krita.la

dummy.cpp:
	echo > dummy.cpp

METASOURCES = AUTO
noinst_HEADERS =  krita_part.h \
	krita_view.h krita_factory.h krita_aboutdata.h

kdelnk_DATA = krita.desktop
kdelnkdir = $(kde_appsdir)/Office

kdemime_DATA = x-krita.desktop
kdemimedir = $(kde_mimedir)/application

rc_DATA = krita.rc
rcdir = $(kde_datadir)/krita

messages: rc.cpp
	$(XGETTEXT) *.cpp krita_aboutdata.h -o $(podir)/krita.pot
