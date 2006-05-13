




########### install files ###############






#original Makefile.am contents follow:

#AM_CPPFLAGS =  -I$(srcdir)/../../../sdk \
#	 -I$(srcdir)/.. \
#	-I$(srcdir)/../../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../../color_strategy/ \
#	$(all_includes)
#
## The check_ target makes sure we don't install the modules,
## $(KDE_CHECK_PLUGIN) assures a shared library is created.
#check_LTLIBRARIES = kunittest_kis_strategy_colorspace_rgb_tester.la
#
#kunittest_kis_strategy_colorspace_rgb_tester_la_SOURCES = kis_strategy_colorspace_rgb_tester.cpp
#kunittest_kis_strategy_colorspace_rgb_tester_la_LIBADD = -lkunittest ../libkritargb.la
#kunittest_kis_strategy_colorspace_rgb_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#check-local: kunittest_kis_strategy_colorspace_rgb_tester.la
#	kunittestmodrunner
#
