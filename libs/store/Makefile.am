
SUBDIRS = . tests

KDE_CXXFLAGS = $(USE_RTTI)
lib_LTLIBRARIES = libkstore.la
INCLUDES = $(all_includes)

####### Files

libkstore_la_LIBADD = $(LIB_KIO)
libkstore_la_SOURCES = KoStore.cpp KoTarStore.cpp KoDirectoryStore.cpp KoZipStore.cpp KoStoreDrag.cpp KoStoreBase.cpp KoXmlWriter.cpp
libkstore_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 $(KDE_LDFLAGS) -no-undefined
include_HEADERS = KoStore.h KoStoreDevice.h KoXmlWriter.h
