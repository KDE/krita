
INCLUDES= $(KOFFICE_INCLUDES) $(KOTEXT_INCLUDES) $(all_includes)

SUBDIRS = pics fonts dtd

####### 
# We have to name it kformulalib, not just kformula, since that's the name of the kdeinit module for kformula.
lib_LTLIBRARIES = libkformulalib.la

libkformulalib_la_SOURCES	= basicelement.cc contextstyle.cc formulacursor.cc \
	formulaelement.cc indexelement.cc kformulacontainer.cc \
	sequenceelement.cc textelement.cc bracketelement.cc \
	matrixelement.cc fractionelement.cc rootelement.cc symbolelement.cc \
	kformulacommand.cc kformulamimesource.cc \
	MatrixDialog.cc sequenceparser.cc elementtype.cc kformuladocument.cc \
	symboltable.cc kformulamathmlread.cc \
	kformulainputfilter.cc kformulaview.cc \
	spaceelement.cc kformulaconfigpage.cc \
	symbolaction.cc cmstyle.cc \
	fontstyle.cc symbolfontstyle.cc esstixfontstyle.cc creationstrategy.cc

#include_HEADERS = kformulacontainer.h kformuladocument.h kformulaview.h \
#	kformuladefs.h kformulaconfigpage.h

libkformulalib_la_LDFLAGS = $(all_libraries) -version-info 4:0 -no-undefined $(KDE_RPATH)
libkformulalib_la_LIBADD  = $(LIB_KDEUI) $(LIB_KOTEXT)

libkformulalib_la_METASOURCES = AUTO

check_PROGRAMS = kformulatest

kformulatest_SOURCES = main.cc kformulawidget.cc
kformulatest_LDADD = libkformulalib.la

#symbolnames.cc: 
#	awk -F, '$$1 !~ "#" {if (split($$3,a," ")>0) print "i18n(\"" a[1] "\");"}' config/unicode.tbl > symbolnames.cc
