INCLUDES  =  -I$(top_srcdir)/krita/sdk \
	-I$(top_srcdir)/krita/core \
	-I$(top_srcdir)/krita/kritacolor/ \
	-I$(top_srcdir)/krita/ui \
	$(KROSS_INCLUDES) \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


lib_LTLIBRARIES = libkritascripting.la
libkritascripting_la_SOURCES = kis_script_progress.cpp kis_script_monitor.cpp
noinst_HEADERS = kis_script_progress.h

libkritascripting_la_LDFLAGS = -no-undefined $(all_libraries)
libkritascripting_la_LIBADD =  $(top_builddir)/krita/libkritacommon.la $(top_builddir)/lib/kross/main/libkrossmain.la

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

libkritascripting_la_METASOURCES = AUTO
