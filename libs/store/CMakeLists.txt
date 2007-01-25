

include_directories( ${KOSTORE_INCLUDES} )

if( QCA2_FOUND )
include_directories( ${QCA2_INCLUDE_DIR} )
add_definitions( -DQCA2 )
endif( QCA2_FOUND )


add_subdirectory( tests )

########### next target ###############

set(kstore_LIB_SRCS 
   KoStore.cpp 
   KoTarStore.cpp 
   KoDirectoryStore.cpp 
   KoZipStore.cpp 
   KoStoreDrag.cpp 
   KoStoreBase.cpp 
   KoXmlReader.cpp
   KoXmlWriter.cpp
   KoEncryptedStore.cpp
   )

kde4_automoc(${kstore_LIB_SRCS})

kde4_add_library(kstore SHARED ${kstore_LIB_SRCS})

target_link_libraries(kstore ${KDE4_KIO_LIBS})

if( QCA2_FOUND )
target_link_libraries(kstore ${QCA2_LIBRARIES})
endif( QCA2_FOUND )

set_target_properties(kstore PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kstore  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  KoStore.h KoStoreDevice.h KoXmlReader.h KoXmlWriter.h kostore_export.h DESTINATION ${INCLUDE_INSTALL_DIR})
