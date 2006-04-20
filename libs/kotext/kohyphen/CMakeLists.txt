kde4_header()

add_subdirectory( hyphdicts )

include_directories( ${KOFFICECORE_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")


########### next target ###############


########### next target ###############

set(kohyphentest_SRCS kohyphentest.cpp )

kde4_automoc(${kohyphentest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kohyphentest ${kohyphentest_SRCS})

target_link_libraries(kohyphentest  ${KDE4_KDECORE_LIBS} kohyphen )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/koffice/hyphdicts FILES  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(all_includes)
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = hnjalloc.h hyphen.h kohyphen.h
#
#noinst_LTLIBRARIES = libkohyphen.la
#libkohyphen_la_SOURCES = hnjalloc.c hyphen.c kohyphen.cpp
#libkohyphen_la_LIBADD = $(LIB_KDECORE)
#libkohyphen_la_LDFLAGS = $(all_libraries) -no-undefined
#
#dictsdir = $(kde_datadir)/koffice/hyphdicts
#
#SUBDIRS = . hyphdicts
#
#check_PROGRAMS = kohyphentest
#
#kohyphentest_SOURCES = kohyphentest.cpp
#kohyphentest_LDADD = ./libkohyphen.la $(LIB_QT) $(LIB_KDECORE)
#kohyphentest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
