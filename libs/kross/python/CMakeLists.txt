

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/kross/api/
	${PYTHON_INCLUDE_PATH}
)



add_subdirectory( cxx )
add_subdirectory( scripts )


set(libkrosspythoncxx_SRCS 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/CXX/cxxsupport.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/CXX/cxx_extensions.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/CXX/cxxextensions.c 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/CXX/IndirectPythonInterface.cxx
	)

########### next target ###############

set(krosspython_PART_SRCS ${libkrosspythoncxx_SRCS}
   pythonextension.cpp 
   pythonmodule.cpp 
   pythonobject.cpp 
   pythonscript.cpp 
   pythonsecurity.cpp 
   pythoninterpreter.cpp )

kde4_automoc(${krosspython_PART_SRCS})

kde4_add_plugin(krosspython ${krosspython_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosspython )

target_link_libraries(krosspython  ${KDE4_KDECORE_LIBS} krossapi krossmain ${PYTHON_LIBRARIES})

install(TARGETS krosspython  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############






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
