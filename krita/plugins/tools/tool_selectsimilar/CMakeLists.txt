kde4_header()

set(kritatoolselectsimilar_PART_SRCS
    selectsimilar.cc
    kis_tool_selectsimilar.cc
    )

kde4_automoc(${kritatoolselectsimilar_PART_SRCS})

kde4_add_plugin(kritatoolselectsimilar ${kritatoolselectsimilar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolselectsimilar )

target_link_libraries(kritatoolselectsimilar  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kritatoolselectsimilar )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/pics FILES  tool_similar_selection.png  	tool_similar_selection_plus_cursor.png  	tool_similar_selection_minus_cursor.png )
install_files( ${SERVICES_INSTALL_DIR} FILES  kritatoolselectsimilar.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritatoolselectsimilar.desktop
#
#INCLUDES  =  -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	-I$/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#
#kritatoolselectsimilar_la_SOURCES = selectsimilar.cc kis_tool_selectsimilar.cc
#noinst_HEADERS = selectsimilar.h kis_tool_selectsimilar.h
#
#kde_module_LTLIBRARIES = kritatoolselectsimilar.la
#
#kritatoolselectsimilar_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritatoolselectsimilar_la_LIBADD =  ../../../libkritacommon.la
#
#kritatoolselectsimilar_la_METASOURCES = AUTO
#
#KDE_OPTIONS = nofinal
#
## directory for pixmaps
#kritapics_DATA = \
#	tool_similar_selection.png \
#	tool_similar_selection_plus_cursor.png \
#	tool_similar_selection_minus_cursor.png
#
#kritapicsdir = $(kde_datadir)/krita/pics
#
