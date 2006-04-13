INCLUDES = -I$(srcdir)/editors -I$(top_srcdir)/libs/kofficecore $(all_includes)

lib_LTLIBRARIES =  libkoproperty.la

libkoproperty_la_LIBADD = $(LIB_KDEUI) ./editors/libkopropertyeditors.la
libkoproperty_la_LDFLAGS = -no-undefined $(all_libraries)
libkoproperty_la_SOURCES = property.cpp customproperty.cpp set.cpp editor.cpp \
	editoritem.cpp factory.cpp widget.cpp

METASOURCES = AUTO

SUBDIRS = editors . test


icondir = $(kde_datadir)/koffice/icons
icon_ICON = button_no

EXTRA_DIST = $(icon_ICON)

messages:
#	$(EXTRACTRC) */*.rc */*.ui > rc.cpp
	$(XGETTEXT) *.cpp editors/*.cpp -o $(podir)/koproperty.pot
