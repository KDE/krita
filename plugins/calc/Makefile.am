
INCLUDES = $(KOFFICE_INCLUDES) $(all_includes) $(MICO_INCLUDES) -I$(idldir)
LDFLAGS	= $(all_libraries) $(MICO_LDFLAGS) $(KDE_RPATH)
LDADD = ../../lib/kofficeui/libkofficeui.la -lm

####### Files

bin_PROGRAMS = kocalc

kocalc_SOURCES = main.cc calc.idl dlabel.cc kcalc.cc configdlg.cc \
	         fontdlg.cc kcalc_core.cc stats.cc kspread.idl

noinst_HEADERS = dlabel.h kcalc.h main.h version.h \
	         configdlg.h fontdlg.h kcalctype.h stats.h 

METASOURCES =	main.moc configdlg.moc dlabel.moc fontdlg.moc kcalc.moc 

service_DATA = kocalc.desktop
servicedir = $(kde_servicesdir)

SUBDIRS = pics

IDL_FLAGS = --query-server-for-narrow --relative-paths --any -I$(top_srcdir)/lib/kofficecore -I$(top_srcdir)/lib/store -I$(idldir)

kspread.idl: $(top_srcdir)/kspread/kspread.idl
	-rm -f kspread.idl
	$(LN_S) $(top_srcdir)/kspread/kspread.idl kspread.idl

