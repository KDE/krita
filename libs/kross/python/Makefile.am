include $(top_srcdir)/lib/kross/Makefile.global

kde_module_LTLIBRARIES = krosspython.la

krosspython_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LIBPYTHON) $(PYTHONLIB) -module $(VER_INFO)

noinst_HEADERS = \
    pythonextension.h \
    pythonmodule.h \
    pythonobject.h \
    pythonscript.h \
    pythonsecurity.h \
    pythoninterpreter.h \
    pythonconfig.h

krosspython_la_SOURCES = \
    pythonextension.cpp \
    pythonmodule.cpp \
    pythonobject.cpp \
    pythonscript.cpp \
    pythonsecurity.cpp \
    pythoninterpreter.cpp

krosspython_la_LIBADD = \
    $(LIB_QT) \
    $(LIB_KDECORE) \
    CXX/libkrosspythoncxx.la \
    ../api/libkrossapi.la \
    ../main/libkrossmain.la

INCLUDES = $(KROSS_INCLUDES) $(PYTHONINC) $(all_includes)
METASOURCES = AUTO
SUBDIRS = CXX scripts .
