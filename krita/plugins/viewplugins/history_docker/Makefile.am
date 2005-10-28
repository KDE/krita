#kde_services_DATA = kritahistorydocker.desktop

INCLUDES  = -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritahistorydocker_la_SOURCES = historydocker.cc 

kde_module_LTLIBRARIES = kritahistorydocker.la
noinst_HEADERS = historydocker.h 

kritahistorydocker_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritahistorydocker_la_LIBADD =  ../../../libkritacommon.la

kritahistorydocker_la_METASOURCES = AUTO
