noinst_HEADERS = test.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

noinst_PROGRAMS = propertytest

propertytest_SOURCES = main.cpp test.cpp
propertytest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
propertytest_LDADD = $(LIB_KOPROPERTY)

# this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/test
#shellrc_DATA = testui.rc

INCLUDES= $(KOPROPERTY_INCLUDES) $(KOFFICE_INCLUDES) $(all_includes) 

