# all_includes must remain last!
INCLUDES  = -I$(srcdir) \
    -I$(srcdir)/../sdk \
    -I$(srcdir)/color_strategy \
 	$(OPENEXR_CFLAGS) \
    $(all_includes)

lib_LTLIBRARIES = libkritacolor.la

libkritacolor_la_SOURCES = \
    kis_color.cc \
    kis_colorspace.cc \
    kis_composite_op.cc \
    kis_profile.cc \
    kis_histogram_producer.cc \
    kis_basic_histogram_producers.cc

libkritacolor_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
libkritacolor_la_LIBADD  = color_strategy/libkiscolor_strategy.la $(LCMS_LIBS) $(LIB_KPARTS) $(LIB_KDECORE) $(LIB_QT) $(OPENEXR_LIBS)

include_HEADERS =  \
    kis_channelinfo.h \
    kis_color.h \
    kis_colorspace.h \
    kis_composite_op.h \
    kis_pixel.h \
    kis_profile.h \
    kis_histogram_producer.h \
    kis_basic_histogram_producers.h

SUBDIRS = color_strategy

kde_servicetypes_DATA =  krita_colorspace.desktop

METASOURCES = AUTO
