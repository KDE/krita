AM_CPPFLAGS = \
	-I$(srcdir)/../ \
	-I$(srcdir)/../resources \
	-I$(srcdir)/../visitors \
	-I$(srcdir)/../tool \
	-I$(srcdir)/../paintop \
	-I$(srcdir)/../tiles \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../../kopalette \
	-I../../ui \
	-I../../kopalette \
	-I$(srcdir)/../../ui/dialogs \
	-I../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/widgets \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) \
	$(KOPAINTER_INCLUDES) \
	-I$(srcdir)/../strategy \
	-I$(srcdir)/../color_strategy \
	-I$(srcdir)/../../modules/rgb_u8 \
	$(all_includes)

# The check_ target makes sure we don't install the modules,
# $(KDE_CHECK_PLUGIN) assures a shared library is created.
check_LTLIBRARIES = kunittest_kis_integer_maths_tester.la kunittest_kis_image_tester.la

kunittest_kis_integer_maths_tester_la_SOURCES = kis_integer_maths_tester.cpp
kunittest_kis_integer_maths_tester_la_LIBADD = -lkunittest ../../libkritacommon.la
kunittest_kis_integer_maths_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

kunittest_kis_image_tester_la_SOURCES = kis_image_tester.cpp
kunittest_kis_image_tester_la_LIBADD = -lkunittest ../../libkritacommon.la
kunittest_kis_image_tester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local: kunittest_kis_integer_maths_tester.la kunittest_kis_image_tester.la
	kunittestmodrunner

