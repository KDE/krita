

include_directories( ${CMAKE_SOURCE_DIR}/libs/kross/ ${CMAKE_SOURCE_DIR}/libs/kross/api/ )
include_directories( ${KOFFICECORE_INCLUDES} )


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

set_target_properties(krossmain PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} krossmain )


########### install files ###############

install_files( /include/kross/main FILES  mainmodule.h      manager.h      scriptaction.h      scriptcontainer.h      scriptguiclient.h      wdgscriptsmanager.h  )


