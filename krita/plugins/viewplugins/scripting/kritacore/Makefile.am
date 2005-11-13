
INCLUDES = -I$(top_srcdir)/core -I$(top_srcdir)/krita -I$(top_srcdir)/lib/kross $(all_includes) -I$(top_srcdir)/krita/ui -I$(top_srcdir)/krita/core -I$(top_srcdir)/krita/sdk

kde_module_LTLIBRARIES = krosskritacore.la

krosskritacore_la_SOURCES = kritacoremodule.cpp krs_doc.cpp krs_layer.cpp \
	krs_image.cpp

krosskritacore_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(VER_INFO) -module
krosskritacore_la_LIBADD = \
    $(LIB_QT) \
    $(LIB_KDECORE) \
    $(top_builddir)/lib/kross/api/libkrossapi.la \
    $(top_builddir)/lib/kross/main/libkrossmain.la \
    $(top_builddir)/krita/libkritacommon.la 

METASOURCES = AUTO
SUBDIRS = .

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_HEADERS = krs_doc.h
