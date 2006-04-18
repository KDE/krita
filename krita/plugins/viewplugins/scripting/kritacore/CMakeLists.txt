kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/core ${CMAKE_SOURCE_DIR}/krita/plugins/viewplugins/scripting/kritascripting ${CMAKE_SOURCE_DIR}/krita ${CMAKE_SOURCE_DIR}/krita/ui ${CMAKE_SOURCE_DIR}/krita/core ${CMAKE_SOURCE_DIR}/krita/sdk ${CMAKE_SOURCE_DIR}/krita/kritacolor ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(krosskritacore_PART_SRCS 
   kritacoremodule.cpp 
   krs_doc.cpp 
   krs_paint_layer.cpp 
   krs_image.cpp 
   krs_histogram.cpp 
   krs_script_progress.cpp 
   krs_painter.cpp 
   krs_color.cpp 
   krs_brush.cpp 
   krs_pattern.cpp 
   krs_filter.cpp 
   krs_filter_configuration.cpp 
   krs_wavelet.cpp )

kde4_automoc(${krosskritacore_PART_SRCS})

kde4_add_plugin(krosskritacore ${krosskritacore_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosskritacore )

target_link_libraries(krosskritacore  ${KDE4_KDECORE_LIBS} krossapi krossmain kritacommon kritascripting )

install_targets(${PLUGIN_INSTALL_DIR} krosskritacore )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(top_srcdir)/core -I$(top_srcdir)/krita/plugins/viewplugins/scripting/kritascripting \
#	-I$(top_srcdir)/krita $(KOFFICECORE_INCLUDES) $(KROSS_INCLUDES) \
#	-I$(top_srcdir)/krita/ui -I$(top_srcdir)/krita/core -I$(top_srcdir)/krita/sdk \
#	-I$(top_srcdir)/krita/kritacolor $(all_includes)
#
#kde_module_LTLIBRARIES = krosskritacore.la
#
#krosskritacore_la_SOURCES = kritacoremodule.cpp krs_doc.cpp krs_paint_layer.cpp \
#	krs_image.cpp krs_histogram.cpp krs_script_progress.cpp krs_painter.cpp krs_color.cpp \
#	krs_brush.cpp krs_pattern.cpp krs_filter.cpp krs_filter_configuration.cpp \
#	krs_wavelet.cpp
#
#krosskritacore_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#krosskritacore_la_LIBADD = \
#    $(LIB_QT) \
#    $(LIB_KDECORE) \
#    $(top_builddir)/lib/kross/api/libkrossapi.la \
#    $(top_builddir)/lib/kross/main/libkrossmain.la \
#    $(top_builddir)/krita/libkritacommon.la \
#    $(top_builddir)/krita/plugins/viewplugins/scripting/kritascripting/libkritascripting.la
#
#METASOURCES = AUTO
#SUBDIRS = .
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_HEADERS = krs_doc.h krs_iterator.h krs_painter.h krs_color.h krs_brush.h \
#	krs_filter.h krs_filter_configuration.h krs_wavelet.h
