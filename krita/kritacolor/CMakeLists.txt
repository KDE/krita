

add_subdirectory( colorspaces )
if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)




########### next target ###############
set(kritacolor_LIB_SRCS 
   kis_histogram_producer.cc 
   kis_basic_histogram_producers.cc 
   kis_color_conversions.cc 
   )
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.cc
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.h

kde4_automoc(${kritacolor_LIB_SRCS})

kde4_add_library(kritacolor SHARED ${kritacolor_LIB_SRCS})

target_link_libraries(kritacolor ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB} pigment kofficecore)

set_target_properties(kritacolor PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritacolor  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  kis_histogram_producer.h      kis_basic_histogram_producers.h  DESTINATION ${INCLUDE_INSTALL_DIR})





#original Makefile.am contents follow:

## all_includes must remain last!
#INCLUDES  = $(KOFFICE_INCLUDES) \
#    -I$(srcdir) \
#    -I$(srcdir)/../sdk \
#    -I$(srcdir)/colorspaces \
#    $(OPENEXR_CFLAGS) \
#    $(all_includes)
#
#lib_LTLIBRARIES = libkritacolor.la
#
#if have_openexr
#OPENEXR_SOURCES=kis_f16half_base_colorspace.cc
#endif
#
#libkritacolor_la_SOURCES = kis_color.cc kis_colorspace.cc \
#	kis_colorspace_iface.cc kis_colorspace_iface.skel kis_composite_op.cc kis_profile.cc \
#	kis_histogram_producer.cc kis_basic_histogram_producers.cc kis_abstract_colorspace.cc \
#	kis_colorspace_factory_registry.cc kis_color_conversions.cc kis_u8_base_colorspace.cc \
#	kis_u16_base_colorspace.cc kis_f32_base_colorspace.cc $(OPENEXR_SOURCES)
#
#libkritacolor_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
#libkritacolor_la_LIBADD  = colorspaces/libkritacolorspaces.la $(LCMS_LIBS) $(LIB_KPARTS) $(LIB_KDECORE) $(LIB_QT) $(OPENEXR_LIBS)
#
#include_HEADERS =  \
#    kis_channelinfo.h \
#    kis_color.h \
#    kis_colorspace.h \
#    kis_composite_op.h \
#    kis_profile.h \
#    kis_histogram_producer.h \
#    kis_basic_histogram_producers.h kis_u8_base_colorspace.h kis_u16_base_colorspace.h kis_f16half_base_colorspace.h kis_f32_base_colorspace.h
#
#
#if include_kunittest_tests
#TESTSDIR = tests
#endif
#
#SUBDIRS = colorspaces . $(TESTSDIR)
#
#kde_servicetypes_DATA =  krita_colorspace.desktop
#
#METASOURCES = AUTO
#
#
