########### next target ###############

set(krosstest_SRCS 
   testobject.cpp 
   testwindow.cpp 
   main.cpp )

kde4_automoc(${krosstest_SRCS})

kde4_add_executable(krosstest ${krosstest_SRCS})

target_link_libraries(krosstest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} krosscore krossmain )


########### install files ###############






#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
#noinst_PROGRAMS = krosstest
#
#krosstest_SOURCES = testobject.cpp testaction.cpp testplugin.cpp testwindow.cpp main.cpp
#
#krosstest_LDADD = \
#    $(LIB_QT) \
#    $(LIB_KDECORE) \
#    $(LIB_KDEUI) \
#    ../api/libkrossapi.la \
#    ../main/libkrossmain.la
#
#INCLUDES = $(KROSS_INCLUDES) $(all_includes)
#krosstest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#SUBDIRS = .
#METASOURCES = AUTO
