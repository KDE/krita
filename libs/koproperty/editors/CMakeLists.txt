kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libs/koproperty ${CMAKE_SOURCE_DIR}/lib/kofficecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(libkopropertyeditors_SRCS
        booledit.cpp
        coloredit.cpp
        combobox.cpp
        cursoredit.cpp
        dateedit.cpp
        datetimeedit.cpp
        dummywidget.cpp
        fontedit.cpp
        linestyledit.cpp
        pixmapedit.cpp
        pointedit.cpp
        rectedit.cpp
        sizeedit.cpp
        sizepolicyedit.cpp
        spinbox.cpp
        stringedit.cpp
        stringlistedit.cpp
        symbolcombo.cpp
        timeedit.cpp
        urledit.cpp
)

kde4_automoc(${libkopropertyeditors_SRCS})

kde4_add_library(kopropertyeditors STATIC ${libkopropertyeditors_SRCS})

target_link_libraries(kopropertyeditors ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/koproperty -I$(top_srcdir)/lib/kofficecore $(all_includes)
#
#noinst_LTLIBRARIES =  libkopropertyeditors.la
#libkopropertyeditors_la_LIBADD =  $(LIB_KDEUI) $(LIB_KIO)
#libkopropertyeditors_la_LDFLAGS = -Wno-unresolved $(all_libraries)
#libkopropertyeditors_la_SOURCES = booledit.cpp coloredit.cpp combobox.cpp cursoredit.cpp dateedit.cpp \
#	datetimeedit.cpp dummywidget.cpp fontedit.cpp linestyledit.cpp pixmapedit.cpp pointedit.cpp \
#	rectedit.cpp sizeedit.cpp sizepolicyedit.cpp spinbox.cpp stringedit.cpp stringlistedit.cpp \
#	symbolcombo.cpp timeedit.cpp urledit.cpp
#
#METASOURCES = AUTO
#
#SUBDIRS = .
