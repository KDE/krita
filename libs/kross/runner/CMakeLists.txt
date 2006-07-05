




########### next target ###############

set(krossrunner_SRCS main.cpp )

kde4_automoc(${krossrunner_SRCS})

kde4_add_executable(krossrunner ${krossrunner_SRCS})

target_link_libraries(krossrunner  ${KDE4_KDECORE_LIBS} krossapi krossmain )

install(TARGETS krossrunner  DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############






#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
##noinst_PROGRAMS = krossrunner
#bin_PROGRAMS = krossrunner
#
#krossrunner_SOURCES = main.cpp
#krossrunner_LDADD = $(LIB_QT) $(LIB_KDECORE) ../api/libkrossapi.la ../main/libkrossmain.la
#INCLUDES = $(KROSS_INCLUDES) $(all_includes)
#krossrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#SUBDIRS = .
#METASOURCES = AUTO
