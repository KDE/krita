




########### next target ###############


########### install files ###############






#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
#noinst_LTLIBRARIES = libkrosspythoncxx.la
#
#libkrosspythoncxx_la_SOURCES = \
#    cxxsupport.cxx \
#    cxx_extensions.cxx \
#    cxxextensions.c \
#    IndirectPythonInterface.cxx
#
#libkrosspythoncxx_la_LDFLAGS = $(LIBPYTHON) $(all_libraries) -Wnounresolved
#
#METASOURCES = AUTO
#INCLUDES = $(KROSS_INCLUDES) $(PYTHONINC) $(all_includes)
#SUBDIRS = .
#
#clean:
#	@rm -f *.o 2> /dev/null
#	@rm -f $(BIN) 2> /dev/null
