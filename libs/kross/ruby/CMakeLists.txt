

include_directories( ${RUBY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/libs/kross )


########### next target ###############

set(krossruby_PART_SRCS
   rubyvariant.cpp
   rubyinterpreter.cpp 
   rubyextension.cpp 
   rubyscript.cpp 
   rubymodule.cpp
   rubycallcache.cpp
   )

kde4_automoc(${krossruby_PART_SRCS})

kde4_add_plugin(krossruby ${krossruby_PART_SRCS})



target_link_libraries(krossruby ${RUBY_LIBRARY} ${KDE4_KROSSCORE_LIBS} )

install(TARGETS krossruby  DESTINATION ${PLUGIN_INSTALL_DIR})

