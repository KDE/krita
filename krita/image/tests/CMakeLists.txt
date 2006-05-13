




########### install files ###############






#original Makefile.am contents follow:

#AM_CPPFLAGS = \
#	-I$(srcdir)/../ \
#	-I$(srcdir)/../tiles \
#	-I$(srcdir)/../../sdk \
#	-I$(srcdir)/../../kritacolor \
#	-I$(srcdir)/../../colorspaces/rgb_u8 \
#	$(KOFFICE_INCLUDES) \
#	$(KOPAINTER_INCLUDES) \
#	$(all_includes)
#
## The check_ target makes sure we don't install the modules,
## $(KDE_CHECK_PLUGIN) assures a shared library is created.
#check_LTLIBRARIES = kunittest_kis_integer_maths_tester.la kunittest_kis_image_tester.la kunittest_kis_filter_configuration_tester.la
#
#kunittest_kis_integer_maths_tester_la_SOURCES = kis_integer_maths_tester.cpp
#kunittest_kis_integer_maths_tester_la_LIBADD = -lkunittest ../../libkritacommon.la
#kunittest_kis_integer_maths_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#kunittest_kis_image_tester_la_SOURCES = kis_image_tester.cpp
#kunittest_kis_image_tester_la_LIBADD = -lkunittest ../../libkritacommon.la
#kunittest_kis_image_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#
#kunittest_kis_filter_configuration_tester_la_SOURCES = kis_filter_configuration_tester.cc
#kunittest_kis_filter_configuration_tester_la_LIBADD = -lkunittest ../../libkritacommon.la
#kunittest_kis_filter_configuration_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#
#check-local: kunittest_kis_integer_maths_tester.la kunittest_kis_image_tester.la kunittest_kis_filter_configuration_tester.la
#	kunittestmodrunner
