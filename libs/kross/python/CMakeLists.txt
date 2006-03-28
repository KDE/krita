kde4_header()

add_subdirectory( CXX )
add_subdirectory( scripts )




########### next target ###############

set(krosspython_PART_SRCS 
   pythonextension.cpp 
   pythonmodule.cpp 
   pythonobject.cpp 
   pythonscript.cpp 
   pythonsecurity.cpp 
   pythoninterpreter.cpp )

kde4_automoc(${krosspython_PART_SRCS})

kde4_add_plugin(krosspython ${krosspython_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosspython )

target_link_libraries(krosspython  ${KDE4_KDECORE_LIBS} krosspythoncxx krossapi krossmain )

install_targets(${LIB_INSTALL_DIR}/kde4 krosspython )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
#kde_module_LTLIBRARIES = krosspython.la
#
#krosspython_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LIBPYTHON) $(PYTHONLIB) -module $(VER_INFO)
#
#noinst_HEADERS = \
#    pythonextension.h \
#    pythonmodule.h \
#    pythonobject.h \
#    pythonscript.h \
#    pythonsecurity.h \
#    pythoninterpreter.h \
#    pythonconfig.h
#
#krosspython_la_SOURCES = \
#    pythonextension.cpp \
#    pythonmodule.cpp \
#    pythonobject.cpp \
#    pythonscript.cpp \
#    pythonsecurity.cpp \
#    pythoninterpreter.cpp
#
#krosspython_la_LIBADD = \
#    $(LIB_QT) \
#    $(LIB_KDECORE) \
#    CXX/libkrosspythoncxx.la \
#    ../api/libkrossapi.la \
#    ../main/libkrossmain.la
#
#INCLUDES = $(KROSS_INCLUDES) $(PYTHONINC) $(all_includes)
#METASOURCES = AUTO
#SUBDIRS = CXX scripts .
