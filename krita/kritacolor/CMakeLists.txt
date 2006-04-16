kde4_header()

add_subdirectory( colorspaces )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TESTSDIR)")



########### next target ###############
set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
	set(FILE_OPENEXR_SOURCES kis_f16half_base_colorspace.cc) 
	include_directories(${OPENEXR_INCLUDE_DIR})
	set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
endif(OPENEXR_FOUND)


set(kritacolor_LIB_SRCS 
   kis_color.cc 
   kis_colorspace.cc 
   kis_colorspace_iface.cc 
   kis_composite_op.cc 
   kis_profile.cc 
   kis_histogram_producer.cc 
   kis_basic_histogram_producers.cc 
   kis_abstract_colorspace.cc 
   kis_colorspace_factory_registry.cc 
   kis_color_conversions.cc 
   kis_u8_base_colorspace.cc 
   kis_u16_base_colorspace.cc 
   kis_f32_base_colorspace.cc 
	${FILE_OPENEXR_SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_alpha_colorspace.cc
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_alpha_colorspace.h
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_lab_colorspace.cc
   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_lab_colorspace.h
   )
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.cc
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.h

kde4_automoc(${kritacolor_LIB_SRCS})

kde4_add_dcop_skels(kritacolor_LIB_SRCS kis_colorspace_iface.h )

kde4_add_library(kritacolor SHARED ${kritacolor_LIB_SRCS})

target_link_libraries(kritacolor ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB})

set_target_properties(kritacolor PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kritacolor )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES  krita_colorspace.desktop )
install_files( /include FILES  kis_channelinfo.h      kis_color.h      kis_colorspace.h      kis_composite_op.h      kis_profile.h      kis_histogram_producer.h      kis_basic_histogram_producers.h kis_u8_base_colorspace.h kis_u16_base_colorspace.h kis_f16half_base_colorspace.h kis_f32_base_colorspace.h )

kde4_footer()



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
