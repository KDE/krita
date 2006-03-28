kde4_header()




########### next target ###############

set(krossmain_LIB_SRCS 
   mainmodule.cpp 
   scriptcontainer.cpp 
   manager.cpp 
   scriptaction.cpp 
   scriptguiclient.cpp 
   wdgscriptsmanager.cpp )

kde4_automoc(${krossmain_LIB_SRCS})

kde4_add_ui3_files(krossmain_LIB_SRCS wdgscriptsmanagerbase.ui )

kde4_add_library(krossmain SHARED ${krossmain_LIB_SRCS})

target_link_libraries(krossmain  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF_LIBS} krossapi )

set_target_properties(krossmain PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} krossmain )


########### install files ###############

install_files( /include/kross/main FILES  krossconfig.h      mainmodule.h      manager.h      scriptaction.h      scriptcontainer.h      scriptguiclient.h      wdgscriptsmanager.h      wdgscriptsmanagerbase.h  )

kde4_footer()



#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
#lib_LTLIBRARIES = libkrossmain.la
#
#libkrossmain_la_SOURCES = mainmodule.cpp scriptcontainer.cpp manager.cpp \
#	scriptaction.cpp scriptguiclient.cpp wdgscriptsmanagerbase.ui wdgscriptsmanager.cpp
#
#libkrossmain_la_LDFLAGS = $(all_libraries) $(VER_INFO) -Wnounresolved
#
#mainincludedir=$(includedir)/kross/main
#
#maininclude_HEADERS = \
#    krossconfig.h \
#    mainmodule.h \
#    manager.h \
#    scriptaction.h \
#    scriptcontainer.h \
#    scriptguiclient.h \
#    wdgscriptsmanager.h \
#    wdgscriptsmanagerbase.h 
#
#libkrossmain_la_LIBADD = \
#    $(LIB_QT) \
#    $(LIB_KDECORE) \
#    $(LIB_KFILE) \
#    $(LIB_KDEUI) \
#    $(LIB_KNEWSTUFF) \
#    $(LIB_KROSS_API)
#
#METASOURCES = AUTO
#SUBDIRS = .
#INCLUDES = $(KROSS_INCLUDES) $(all_includes)
#noinst_HEADERS = wdgscriptsmanager.h
