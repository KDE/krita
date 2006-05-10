kde4_header()




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(srcdir)/../ \
#	-I$(srcdir)/../.. \
#	-I$(srcdir)/../../../sdk \
#	$(all_includes)
#
## The check_ target makes sure we don't install the modules,
## $(KDE_CHECK_PLUGIN) assures a shared library is created.
#check_LTLIBRARIES = kunittest_kis_tiled_data_tester.la
#kunittest_kis_tiled_data_tester_la_SOURCES = kis_tiled_data_tester.cpp
#kunittest_kis_tiled_data_tester_la_LIBADD = -lkunittest ../../../libkritacommon.la
#kunittest_kis_tiled_data_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#check-local: kunittest_kis_tiled_data_tester.la
#	kunittestmodrunner
#
