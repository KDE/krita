AM_CPPFLAGS = \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../ \
	$(all_includes)

# The check_ target makes sure we don't install the modules,
# $(KDE_CHECK_PLUGIN) assures a shared library is created.
check_LTLIBRARIES = kunittest_kis_color_conversions_tester.la

kunittest_kis_color_conversions_tester_la_SOURCES = kis_color_conversions_tester.cpp
kunittest_kis_color_conversions_tester_la_LIBADD = -lkunittest ../libkritacolor.la ../../libkritacommon.la
kunittest_kis_color_conversions_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local: kunittest_kis_color_conversions_tester.la
	kunittestmodrunner

