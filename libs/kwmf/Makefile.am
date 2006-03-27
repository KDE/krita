####### General stuff

INCLUDES= $(KOFFICECORE_INCLUDES) -I$(srcdir) $(all_includes)
libkwmf_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 -no-undefined
# We use kdecore for kdDebug :)
libkwmf_la_LIBADD = $(LIB_KDECORE)

####### Files

lib_LTLIBRARIES = libkwmf.la libkowmf.la

libkwmf_la_SOURCES = kwmf.cc qwmf.cc

#include_HEADERS = kowmfread.h kowmfwrite.h kowmfpaint.h
noinst_HEADERS = kwmf.h qwmf.h metafuncs.h wmfstruct.h kowmfreadprivate.h kowmfstack.h kowmfstruct.h

libkowmf_la_SOURCES  = kowmfreadprivate.cc kowmfstack.cc kowmfread.cc kowmfwrite.cc kowmfpaint.cc
libkowmf_la_LDFLAGS = $(all_libraries) -version-info 2:0:0  -no-undefined
libkowmf_la_LIBADD = $(LIB_KDECORE)
