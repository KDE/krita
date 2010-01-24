include_directories( ${KOSTORE_INCLUDES} )

if( QCA2_FOUND )
    include_directories( ${QCA2_INCLUDE_DIR} )
    add_definitions( -DQCA2 )
endif( QCA2_FOUND )

add_subdirectory( tests )

########### next target ###############

set(kostore_LIB_SRCS 
   KoStore.cpp
   KoTarStore.cpp
   KoDirectoryStore.cpp
   KoZipStore.cpp
   KoXmlReader.cpp
   KoXmlWriter.cpp
   KoEncryptedStore.cpp
   KoEncryptionChecker.cpp
   )

kde4_add_library(kostore SHARED ${kostore_LIB_SRCS})

target_link_libraries(kostore ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})
target_link_libraries(kostore LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})

if( QCA2_FOUND )
target_link_libraries(kostore ${QCA2_LIBRARIES})
endif( QCA2_FOUND )

set_target_properties(kostore PROPERTIES
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kostore  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES  KoStore.h KoStoreDevice.h KoXmlReader.h KoXmlWriter.h kostore_export.h KoXmlReaderForward.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
