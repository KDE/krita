INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../core/tiles -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) $(all_includes) $(KOPAINTER_INCLUDES) 


LDFLAGS = $(all_libraries) $(KDE_RPATH) 
LDADD =  $(LIB_KDEUI) ../core/libkiscore.la ../core/tiles/libkistile.la

bin_PROGRAMS = kis_mutex_test tilestest

tilestest_SOURCES = tiles.cpp

kis_mutex_test_SOURCES = kis_mutex_test.cpp

