
INCLUDES = $(KOFFICE_INCLUDES) $(all_includes) $(MICO_INCLUDES) -I$(idldir)
LDFLAGS	= $(all_libraries) $(MICO_LDFLAGS) $(KDE_RPATH)
LDADD = ../../lib/kofficeui/libkofficeui.la -lm

####### Files

bin_PROGRAMS = kocalc

kocalc_SOURCES = main.cc calc.cc dlabel.cc kcalc.cc configdlg.cc \
	         fontdlg.cc kcalc_core.cc stats.cc kspread.cc

noinst_HEADERS = calc.h dlabel.h kcalc.h main.h version.h \
	         configdlg.h fontdlg.h kcalctype.h stats.h kspread.h

METASOURCES =	main.moc configdlg.moc dlabel.moc fontdlg.moc kcalc.moc 

DISTCLEANFILES = $(idl_DATA:.idl=.cc) $(idl_HEADERS) $(METASOURCES) calc.h calc.cc \
	         kspread.cc kspread.h

idl_DATA = 
idl_HEADERS = 

kdelnk_DATA = kocalc.kdelnk
kdelnkdir = $(kde_datadir)/koffice/pluginlnk

SUBDIRS = pics

####### Build rules

IDL_FLAGS = --query-server-for-narrow --relative-paths -I$(top_srcdir)/lib/kofficecore -I$(top_srcdir)/lib/store -I$(idldir)

messages: 
#	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot  *.cc && mv messages.po ../../calc.pot

calc.h: calc.idl
	rm -f calc.h
	rm -f calc.cc
	$(IDL) $(IDL_FLAGS) $(srcdir)/calc.idl

kspread.h: $(top_srcdir)/kspread/kspread.idl
	rm -f kspread.h
	rm -f kspread.cc
	$(IDL) $(IDL_FLAGS) $(top_srcdir)/kspread/kspread.idl

main.cc: main.h calc.h kspread.h
calc.cc: calc.h
kspread.cc: kspread.h
