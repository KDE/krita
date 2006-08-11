

include_directories( ${KOSTORE_INCLUDES} )

add_subdirectory( tests )

########### next target ###############

set(kstore_LIB_SRCS 
   KoStore.cpp 
   KoTarStore.cpp 
   KoDirectoryStore.cpp 
   KoZipStore.cpp 
   KoStoreDrag.cpp 
   KoStoreBase.cpp 
   KoXmlWriter.cpp )

kde4_automoc(${kstore_LIB_SRCS})

kde4_add_library(kstore SHARED ${kstore_LIB_SRCS})

target_link_libraries(kstore  ${KDE4_KIO_LIBS} )

set_target_properties(kstore PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kstore  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  KoStore.h KoStoreDevice.h KoXmlWriter.h  DESTINATION ${INCLUDE_INSTALL_DIR})
