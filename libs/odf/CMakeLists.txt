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
    KoFontFace.cpp
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
    KoBorder.cpp
    KoPageLayout.cpp
    KoPageFormat.cpp

    # depracated classes, have to have a home which for now is here.
    kopicture/KoPictureKey.cpp
    kopicture/KoPictureBase.cpp
    kopicture/KoPicture.cpp
    kopicture/KoPictureShared.cpp
    kopicture/KoPictureImage.cpp
    kopicture/KoPictureClipart.cpp
    kopicture/KoPictureEps.cpp
)

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf kostore kobase)
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES kostore kobase)

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
    KoFontFace.h
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
    KoGenChange.h
    KoGenChanges.h
    KoEmbeddedDocumentSaver.h
    KoBorder.h
    KoPageLayout.h
    KoPageFormat.h
    KoColumns.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

