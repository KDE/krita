INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) $(all_includes)

LDFLAGS = $(all_libraries) $(KDE_RPATH) 
LDADD =  $(LIB_KDEUI) ../core/libkiscore.la

bin_PROGRAMS = kis_color_test kis_brush_test

kis_color_test_SOURCES = kis_color_test.cc

kis_brush_test_SOURCES = kis_brush_test.cc
