include_directories( ${KOFFICECORE_INCLUDES} )

########### next target ###############

set(krosscore_LIB_SRCS
   krossconfig.cpp
   object.cpp 
   metaobject.cpp 
   interpreter.cpp 
   script.cpp 
   action.cpp
   manager.cpp
   guiclient.cpp    
   )

kde4_automoc(${krosscore_LIB_SRCS})

kde4_add_library(krosscore SHARED ${krosscore_LIB_SRCS})

#target_link_libraries(krosscore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF_LIBS})
target_link_libraries(krosscore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(krosscore PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krosscore  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

install( FILES krossconfig.h object.h metaobject.h metatype.h interpreter.h script.h action.h manager.h guiclient.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/core)
