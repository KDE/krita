include_directories( ${CMAKE_SOURCE_DIR}/libs/kross/core/ ${KDE4_INCLUDES} )

########### next target ###############

set(krosskjs_PART_SRCS
   kjsscript.cpp 
   kjsinterpreter.cpp
)

kde4_automoc(${krosskjs_PART_SRCS})

kde4_add_plugin(krosskjs ${krosskjs_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosskjs )

#target_link_libraries(krosskjs  ${KDE4_KDECORE_LIBS} krosscore kjs)
target_link_libraries(krosskjs  ${KDE4_KDECORE_LIBS} krosscore kjs kjsembed)

install(TARGETS krosskjs DESTINATION ${PLUGIN_INSTALL_DIR})

