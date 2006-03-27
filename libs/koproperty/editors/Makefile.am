INCLUDES = -I$(top_srcdir)/lib/koproperty -I$(top_srcdir)/lib/kofficecore $(all_includes)

noinst_LTLIBRARIES =  libkopropertyeditors.la
libkopropertyeditors_la_LIBADD =  $(LIB_KDEUI) $(LIB_KIO)
libkopropertyeditors_la_LDFLAGS = -Wno-unresolved $(all_libraries)
libkopropertyeditors_la_SOURCES = booledit.cpp coloredit.cpp combobox.cpp cursoredit.cpp dateedit.cpp \
	datetimeedit.cpp dummywidget.cpp fontedit.cpp linestyledit.cpp pixmapedit.cpp pointedit.cpp \
	rectedit.cpp sizeedit.cpp sizepolicyedit.cpp spinbox.cpp stringedit.cpp stringlistedit.cpp \
	symbolcombo.cpp timeedit.cpp urledit.cpp

METASOURCES = AUTO

SUBDIRS = .
