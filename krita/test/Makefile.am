INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../core/tiles -I$(srcdir)/../ui -I$(srcdir)/../tools $(KOFFICE_INCLUDES) $(all_includes) $(KOPAINTER_INCLUDES) 

bin_PROGRAMS = kis_mutex_test tilestest

tilestest_SOURCES = tiles.cpp
tilestest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
tilestest_LDADD = ../core/libkiscore.la ../core/tiles/libkistile.la $(LIB_KDEUI)

kis_mutex_test_SOURCES = kis_mutex_test.cpp
kis_mutex_test_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kis_mutex_test_LDADD = ../core/libkiscore.la ../core/tiles/libkistile.la $(LIB_KDEUI)


