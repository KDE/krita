INCLUDES  = -I$(srcdir)/../core -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) -I$(interfacedir) $(all_includes)

libkisp_example_la_LDFLAGS = $(all_libraries) -avoid-version -module
#libkisp_example_la_LIBADD =  ../core/libkiscore.la ../ui/libkisui.la ../tools/libkistools.la \
#	../../lib/kofficeui/libkofficeui.la

lib_LTLIBRARIES = libkisp_example.la

libkisp_example_la_SOURCES = example.cc

kisplugin_DATA = example.kisplugin
kisplugindir = $(kde_datadir)/krayon/plugins/

