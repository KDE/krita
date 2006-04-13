include $(top_srcdir)/libs/kross/Makefile.global

INCLUDES = -I$(top_srcdir)/libs/kross -I$(RUBY_INCLUDEDIR) $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = krossruby.la

krossruby_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(RUBY_LIBRUBYARG) -module $(VER_INFO)

krossruby_la_LIBADD = \
    $(LIB_QT) \
    $(LIB_KDECORE) \
    ../api/libkrossapi.la \
    ../main/libkrossmain.la

noinst_HEADERS = rubyinterpreter.h rubyextension.h rubyscript.h rubyconfig.h
krossruby_la_SOURCES = rubyinterpreter.cpp rubyextension.cpp rubyscript.cpp rubymodule.cpp
