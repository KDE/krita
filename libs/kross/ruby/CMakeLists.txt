

include_directories( ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/lib/kross ${KDE4_INCLUDES} )


########### next target ###############

set(krossruby_PART_SRCS 
   rubyinterpreter.cpp 
   rubyextension.cpp 
   rubyscript.cpp 
   rubymodule.cpp )

kde4_automoc(${krossruby_PART_SRCS})

kde4_add_plugin(krossruby ${krossruby_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krossruby )

target_link_libraries(krossruby  ${KDE4_KDECORE_LIBS} ${RUBY_LIBRARY} krossapi krossmain )

install(TARGETS krossruby  DESTINATION ${PLUGIN_INSTALL_DIR})

