kde4_header()




########### next target ###############

set(krossmain_LIB_SRCS 
   mainmodule.cpp 
   scriptcontainer.cpp 
   manager.cpp 
   scriptaction.cpp 
   scriptguiclient.cpp 
   wdgscriptsmanager.cpp )

kde4_automoc(${krossmain_LIB_SRCS})

kde4_add_library(krossmain SHARED ${krossmain_LIB_SRCS})

target_link_libraries(krossmain  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF_LIBS} krossapi )

set_target_properties(krossmain PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} krossmain )


########### install files ###############

install_files( /include/kross/main FILES  krossconfig.h      mainmodule.h      manager.h      scriptaction.h      scriptcontainer.h      scriptguiclient.h      wdgscriptsmanager.h  )

kde4_footer()
