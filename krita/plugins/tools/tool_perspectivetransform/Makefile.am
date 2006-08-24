kde_services_DATA = kritatoolperspectivetransform.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritatoolperspectivetransform_la_SOURCES = \
	tool_perspectivetransform.cc \
	kis_tool_perspectivetransform.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolperspectivetransform.la

noinst_HEADERS = \
	tool_perspectivetransform.h \
	kis_tool_perspectivetransform.h 

kritatoolperspectivetransform_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolperspectivetransform_la_LIBADD =  ../../../libkritacommon.la

kritatoolperspectivetransform_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_perspectivetransform.png

kritapicsdir = $(kde_datadir)/krita/pics

