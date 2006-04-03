kde4_header()

add_subdirectory( editors )
add_subdirectory( test )

include_directories( ${CMAKE_SOURCE_DIR}/libs/koproperty/editors ${CMAKE_SOURCE_DIR}/libs/kofficecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


set(libkopropertyeditors_SRCS
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/booledit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/coloredit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/combobox.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/cursoredit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/dateedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/datetimeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/dummywidget.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/fontedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/linestyledit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/pixmapedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/pointedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/rectedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/sizeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/sizepolicyedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/spinbox.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/stringedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/stringlistedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/symbolcombo.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/timeedit.cpp
	${CMAKE_SOURCE_DIR}/libs/kopropert/editors/urledit.cpp
)

########### next target ###############

set(koproperty_LIB_SRCS ${libkopropertyeditors_SRC}
   property.cpp
   customproperty.cpp
   set.cpp
   editor.cpp
   editoritem.cpp
   factory.cpp
   widget.cpp )

kde4_automoc(${koproperty_LIB_SRCS})

kde4_add_library(koproperty SHARED ${koproperty_LIB_SRCS})

target_link_libraries(koproperty  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}  )

set_target_properties(koproperty PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} koproperty )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/koffice/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/editors -I$(top_srcdir)/libs/kofficecore $(all_includes)
#
#lib_LTLIBRARIES =  libkoproperty.la
#
#libkoproperty_la_LIBADD = $(LIB_KDEUI) ./editors/libkopropertyeditors.la
#libkoproperty_la_LDFLAGS = -no-undefined $(all_libraries)
#libkoproperty_la_SOURCES = property.cpp customproperty.cpp set.cpp editor.cpp \
#	editoritem.cpp factory.cpp widget.cpp
#
#METASOURCES = AUTO
#
#SUBDIRS = editors . test
#
#
#icondir = $(kde_datadir)/koffice/icons
#icon_ICON = button_no
#
#EXTRA_DIST = $(icon_ICON)
#
#messages:
##	$(EXTRACTRC) */*.rc */*.ui > rc.cpp
#	$(XGETTEXT) *.cpp editors/*.cpp -o $(podir)/koproperty.pot
