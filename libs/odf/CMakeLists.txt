add_subdirectory( tests )

include_directories( ${KOSTORE_INCLUDES} )

########### libkoodf ###############

#if( QCA2_FOUND )
#add_definitions( -DQCA2 )
#endif( QCA2_FOUND )

set(koodf_LIB_SRCS
    KoOdf.cpp
    KoXmlNS.cpp
    KoDom.cpp
    KoUnit.cpp
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
)

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf kstore)

set_target_properties(koodf PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    koodf_export.h
    KoOdf.h
    KoXmlNS.h
    KoDom.h
    KoUnit.h
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
DESTINATION ${INCLUDE_INSTALL_DIR})

