
INCLUDES = $(KOFFICE_INCLUDES) $(all_includes) $(MICO_INCLUDES) -I$(idldir)
LDFLAGS	= $(all_libraries) $(MICO_LDFLAGS) $(KDE_RPATH)
LDADD = -lkparts -lkpartsui -lkom $(LIB_KIMGIO) \
	-lkspell -lkdeutil $(LIBMICO) $(LIB_KFILE) 

####### Files

bin_PROGRAMS = makro_recorder

makro_recorder_SOURCES = main.cc recorder.cc

noinst_HEADERS = main.h recorder.h

SRCMETA =	main.moc

DISTCLEANFILES = $(idl_DATA:.idl=.cc) $(idl_HEADERS) $(SRCMETA)

idl_DATA = 
idl_HEADERS =  

kdelnk_DATA = recorder.kdelnk
kdelnkdir = $(kde_datadir)/koffice/pluginlnk

####### Implicit rules

SUFFIXES = .h .moc

.h.moc:
	$(MOC) $< -o $@      

####### Build rules

install-exec-local:
	$(INSTALL_SCRIPT) makro_recorder $(bindir)/makro_recorder

messages: 
#	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot  *.cc && mv messages.po ../../recorder.pot

recorder.h: recorder.idl
	rm -f recorder.h
	rm -f recorder.cc
	$(IDL) --query-server-for-narrow --relative-paths -I$(idldir) recorder.idl

main.cc: main.h recorder.h
recorder.cc: recorder.h
main.cc: main.moc


