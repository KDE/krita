
INCLUDES = $(KOFFICE_INCLUDES) $(all_includes) $(MICO_INCLUDES) -I$(idldir)
LDFLAGS	= $(all_libraries) $(MICO_LDFLAGS) $(KDE_RPATH)
LDADD = -lkom $(top_builddir)/lib/kofficecore/libkofficecore.la $(LIBMICO)

####### Files

bin_PROGRAMS = makro_recorder

makro_recorder_SOURCES = main.cc recorder.idl

noinst_HEADERS = main.h 
METASOURCES =	main.moc

service_DATA = recorder.desktop
servicedir = $(kde_servicesdir)

SUBDIRS = pics

messages:
#	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot  *.cc && mv messages.po ../../recorder.pot

IDL_FLAGS = --query-server-for-narrow --relative-paths -I$(idldir) 

