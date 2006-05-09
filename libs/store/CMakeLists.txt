kde4_header()

include_directories( ${KOSTORE_INCLUDES} )

add_subdirectory( tests )

########### next target ###############

set(kstore_LIB_SRCS 
   KoStore.cpp 
   KoTarStore.cpp 
   KoDirectoryStore.cpp 
   KoZipStore.cpp 
   KoStoreDrag.cpp 
   KoStoreBase.cpp 
   KoXmlWriter.cpp )

kde4_automoc(${kstore_LIB_SRCS})

kde4_add_library(kstore SHARED ${kstore_LIB_SRCS})

target_link_libraries(kstore  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )

set_target_properties(kstore PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install_targets(${LIB_INSTALL_DIR} kstore )


########### install files ###############

install_files( /include FILES  KoStore.h KoStoreDevice.h KoXmlWriter.h )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = . tests
#
#KDE_CXXFLAGS = $(USE_RTTI)
#lib_LTLIBRARIES = libkstore.la
#INCLUDES = $(all_includes)
#
######## Files
#
#libkstore_la_LIBADD = $(LIB_KIO)
#libkstore_la_SOURCES = KoStore.cpp KoTarStore.cpp KoDirectoryStore.cpp KoZipStore.cpp KoStoreDrag.cpp KoStoreBase.cpp KoXmlWriter.cpp
#libkstore_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 $(KDE_LDFLAGS) -no-undefined
#include_HEADERS = KoStore.h KoStoreDevice.h KoXmlWriter.h
