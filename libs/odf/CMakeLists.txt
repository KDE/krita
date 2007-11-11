#add_subdirectory( tests )

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
    KoOdfWriteStore.cpp
    KoStyleStack.cpp
)

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf kstore)

set_target_properties(koodf PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koodf DESTINATION ${LIB_INSTALL_DIR} )


install( FILES
    KoOdf.h
    KoXmlNS.h
    KoDom.h
    KoUnit.h
    KoGenStyle.h
    KoGenStyles.h
    KoOdfWriteStore.h
    KoStyleStack.h
DESTINATION ${INCLUDE_INSTALL_DIR})

