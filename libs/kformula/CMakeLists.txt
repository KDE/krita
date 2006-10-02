

add_subdirectory( pics ) 
add_subdirectory( fonts ) 

include_directories( ${KOTEXT_INCLUDES} ${KOFFICEUI_INCLUDES})


########### next target ###############

set(kformulalib_LIB_SRCS 
   FormulaShape.cpp
   FormulaTool.cpp
   FormulaCursor.cpp 
   FormulaContainer.cpp
   BasicElement.cpp 
   SequenceElement.cpp
   FormulaElement.cpp 
   MultiscriptElement.cpp
   UnderOverElement.cpp
   TextElement.cpp 
   BracketElement.cpp 
   MatrixElement.cpp
   MatrixRowElement.cpp
   MatrixEntryElement.cpp
   FractionElement.cpp
   RootElement.cpp
   SpaceElement.cpp 
#   ActionElement.cpp
#   PaddedElement.cpp
#   GlyphElement.cpp
#   ErrorElement.cpp
   ElementFactory.cpp 
#   kformulacommand.cc 
   MatrixDialog.cc 
   contextstyle.cc 
   symboltable.cc 
   kformulaconfigpage.cc 
   symbolaction.cc 
   cmstyle.cc 
   fontstyle.cc 
   symbolfontstyle.cc 
   esstixfontstyle.cc )

kde4_automoc(${kformulalib_LIB_SRCS})

kde4_add_library(kformulalib SHARED ${kformulalib_LIB_SRCS})

target_link_libraries(kformulalib  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kotext kofficeui)

set_target_properties(kformulalib PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kformulalib  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

#original Makefile.am contents follow:

#
#INCLUDES= $(KOFFICE_INCLUDES) $(KOTEXT_INCLUDES) $(all_includes)
#
#SUBDIRS = pics fonts dtd
#
######## 
## We have to name it kformulalib, not just kformula, since that's the name of the kdeinit module for kformula.
#lib_LTLIBRARIES = libkformulalib.la
#
#libkformulalib_la_SOURCES	= basicelement.cc contextstyle.cc formulacursor.cc \
#	formulaelement.cc indexelement.cc kformulacontainer.cc \
#	sequenceelement.cc textelement.cc bracketelement.cc \
#	matrixelement.cc fractionelement.cc rootelement.cc symbolelement.cc \
#	kformulacommand.cc kformulamimesource.cc \
#	MatrixDialog.cc sequenceparser.cc elementtype.cc kformuladocument.cc \
#	symboltable.cc kformulamathmlread.cc \
#	kformulainputfilter.cc kformulaview.cc \
#	spaceelement.cc kformulaconfigpage.cc \
#	symbolaction.cc cmstyle.cc \
#	fontstyle.cc symbolfontstyle.cc esstixfontstyle.cc creationstrategy.cc
#
##include_HEADERS = kformulacontainer.h kformuladocument.h kformulaview.h \
##	kformuladefs.h kformulaconfigpage.h
#
#libkformulalib_la_LDFLAGS = $(all_libraries) -version-info 4:0 -no-undefined $(KDE_RPATH)
#libkformulalib_la_LIBADD  = $(LIB_KDEUI) $(LIB_KOTEXT)
#
#libkformulalib_la_METASOURCES = AUTO
#
#check_PROGRAMS = kformulatest
#
#kformulatest_SOURCES = main.cc kformulawidget.cc
#kformulatest_LDADD = libkformulalib.la
#
##symbolnames.cc: 
##	awk -F, '$$1 !~ "#" {if (split($$3,a," ")>0) print "i18n(\"" a[1] "\");"}' config/unicode.tbl > symbolnames.cc
