




########### install files ###############






#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(srcdir)/.. \
#	-I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../kritacolor/color_strategy/ \
#	-I$(srcdir)/../../../color_strategy/ \
#	-I$(srcdir)/../../../modules/rgb_f32 \
#	$(OPENEXR_CFLAGS) \
#	$(all_includes)
#
## The check_ target makes sure we don't install the modules,
## $(KDE_CHECK_PLUGIN) assures a shared library is created.
#check_LTLIBRARIES = kunittest_kis_rgb_f16half_colorspace_tester.la
#
#kunittest_kis_rgb_f16half_colorspace_tester_la_SOURCES = kis_rgb_f16half_colorspace_tester.cc
#kunittest_kis_rgb_f16half_colorspace_tester_la_LIBADD = -lkunittest ../libkrita_rgb_f16half.la
#kunittest_kis_rgb_f16half_colorspace_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#check-local: kunittest_kis_rgb_f16half_colorspace_tester.la
#	kunittestmodrunner
#
