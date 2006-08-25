

include_directories( ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/libs/kross ${KDE4_INCLUDES} )


########### next target ###############

set(krossruby_PART_SRCS
   rubyvariant.cpp
   rubyinterpreter.cpp 
   rubyextension.cpp 
   rubyscript.cpp 
   rubymodule.cpp )

kde4_automoc(${krossruby_PART_SRCS})

kde4_add_plugin(krossruby ${krossruby_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krossruby )

target_link_libraries(krossruby  ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY} krosscore )

install(TARGETS krossruby  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############






#original Makefile.am contents follow:

#include $(top_srcdir)/lib/kross/Makefile.global
#
#INCLUDES = -I$(top_srcdir)/lib/kross -I$(RUBY_ARCHDIR) $(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = krossruby.la
#
#krossruby_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(RUBY_LIBRUBYARG) -module $(VER_INFO)
#
#krossruby_la_LIBADD = \
#    $(LIB_QT) \
#    $(LIB_KDECORE) \
#    ../api/libkrossapi.la \
#    ../main/libkrossmain.la
#
#noinst_HEADERS = rubyinterpreter.h rubyextension.h rubyscript.h rubyconfig.h
#krossruby_la_SOURCES = rubyinterpreter.cpp rubyextension.cpp rubyscript.cpp rubymodule.cpp
