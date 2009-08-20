add_subdirectory( tests )

include_directories( ${KOODF_INCLUDES} )

########### libkoodf ###############

#if( QCA2_FOUND )
#add_definitions( -DQCA2 )
#endif( QCA2_FOUND )

set(koodf_LIB_SRCS
    KoOdf.cpp
    KoXmlNS.cpp
    KoDom.cpp
    KoGenStyle.cpp
    KoGenStyles.cpp
    KoOdfLoadingContext.cpp
    KoOasisSettings.cpp
    KoOdfStylesReader.cpp
    KoOdfNumberStyles.cpp
    KoOdfPaste.cpp
    KoOdfReadStore.cpp
    KoOdfWriteStore.cpp
    KoStyleStack.cpp
    KoOdfGraphicStyles.cpp
    KoGenChange.cpp
    KoGenChanges.cpp
    KoOdfDocument.cpp
    KoEmbeddedDocumentSaver.cpp
)

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf kostore kopicture kobase)
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES kostore kopicture kobase)

set_target_properties(koodf PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    koodf_export.h
    KoOdf.h
    KoXmlNS.h
    KoDom.h
    KoGenStyle.h
    KoGenStyles.h
    KoOdfLoadingContext.h
    KoOasisSettings.h
    KoOdfStylesReader.h
    KoOdfNumberStyles.h
    KoOdfPaste.h
    KoOdfReadStore.h
    KoOdfWriteStore.h
    KoStyleStack.h
    KoOdfGraphicStyles.h
    KoOdfDocument.h
    KoEmbeddedDocumentSaver.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if(CHANGETRK)
  install( FILES ${FILES}
    KoGenChange.h
    KoGenChanges.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif(CHANGETRK)

