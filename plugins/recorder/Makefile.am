
INCLUDES = $(KOFFICE_INCLUDES) $(all_includes) $(MICO_INCLUDES) -I$(idldir)
LDFLAGS	= $(all_libraries) $(MICO_LDFLAGS) $(KDE_RPATH)
LDADD = -lkom $(top_builddir)/lib/kofficecore/libkofficecore.la $(LIBMICO)

####### Files

bin_PROGRAMS = makro_recorder

makro_recorder_SOURCES = main.cc recorder.cc

noinst_HEADERS = main.h recorder.h

METASOURCES =	main.moc

DISTCLEANFILES = $(idl_DATA:.idl=.cc) $(idl_HEADERS) $(METASOURCES) recorder.h recorder.cc

idl_DATA =
idl_HEADERS =

kdelnk_DATA = recorder.kdelnk
kdelnkdir = $(kde_datadir)/koffice/pluginlnk

SUBDIRS = pics

####### Implicit rules


####### Build rules

messages:
#	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot  *.cc && mv messages.po ../../recorder.pot

recorder.h: recorder.idl
	rm -f recorder.h
	rm -f recorder.cc
	$(IDL) --query-server-for-narrow --relative-paths -I$(idldir) $(srcdir)/recorder.idl

main.cc: main.h recorder.h
recorder.cc: recorder.h


