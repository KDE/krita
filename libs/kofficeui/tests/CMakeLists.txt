kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir)/.. $(KOFFICECORE_INCLUDES) $(all_includes)
#
######## Files
#
#METASOURCES = AUTO
#
#check_PROGRAMS = # coloraction_test
#
#TESTS =
#
##coloraction_test_SOURCES = coloraction_test.cpp
##coloraction_test_LDADD = ../libkofficeui.la
#
