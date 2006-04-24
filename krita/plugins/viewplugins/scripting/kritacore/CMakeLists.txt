kde4_header()

#create the moc files and add them to the list of sources
#usage: KDE_ADD_MOC_FILES(foo_SRCS ${moc_headers})
MACRO(tmp_ADD_MOC_FILES _sources)
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)
      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_tmp_FILE} -o ${_moc}
         DEPENDS ${_tmp_FILE}
      )

      SET(${_sources} ${${_sources}} ${_moc})

   ENDFOREACH (_current_FILE)
ENDMACRO(tmp_ADD_MOC_FILES)


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
   krs_wavelet.cpp
   )

kde4_automoc(${krosskritacore_PART_SRCS})

tmp_add_moc_files(krosskritacore_PART_SRCS krs_iterator.h)

kde4_add_plugin(krosskritacore ${krosskritacore_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosskritacore )

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

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
