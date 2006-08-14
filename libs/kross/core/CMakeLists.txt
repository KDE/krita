include_directories( ${KOFFICECORE_INCLUDES} )

########### next target ###############

set(krosscore_LIB_SRCS
   krossconfig.cpp
   object.cpp 
   metaobject.cpp 
   interpreter.cpp 
   script.cpp 
   action.cpp
   manager.cpp
   guiclient.cpp    
   )

kde4_automoc(${krosscore_LIB_SRCS})

kde4_add_library(krosscore SHARED ${krosscore_LIB_SRCS})

#target_link_libraries(krosscore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF_LIBS})
target_link_libraries(krosscore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

set_target_properties(krosscore PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krosscore  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

install( FILES krossconfig.h object.h metaobject.h interpreter.h script.h action.h manager.h guiclient.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/core)





#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#apiincludedir=$(includedir)/kross/api
#
#apiinclude_HEADERS = \
#    callable.h \
#    class.h \
#    dict.h \
#    eventaction.h \
#    event.h \
#    eventscript.h \
#    eventsignal.h \
#    eventslot.h \
#    exception.h \
#    function.h \
#    interpreter.h \
#    list.h \
#    module.h \
#    object.h \
#    proxy.h \
#    qtobject.h \
#    script.h \
#    value.h \
#    variant.h
#
#lib_LTLIBRARIES = libkrossapi.la
#
#libkrossapi_la_SOURCES = \
#    object.cpp \
#    variant.cpp \
#    list.cpp \
#    dict.cpp \
#    exception.cpp \
#    callable.cpp \
#    eventaction.cpp \
#    eventsignal.cpp \
#    eventslot.cpp \
#    eventscript.cpp \
#    qtobject.cpp \
#    script.cpp \
#    interpreter.cpp
#
#libkrossapi_la_LDFLAGS = $(all_libraries) $(VER_INFO) -Wnounresolved
#libkrossapi_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#METASOURCES = AUTO
#SUBDIRS = .
#INCLUDES = $(KROSS_INCLUDES) $(all_includes)
