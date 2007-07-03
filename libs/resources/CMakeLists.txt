include_directories( ${KOMAIN_INCLUDES} ${PIGMENT_INCLUDES} )


########### next target ###############

set(koresources_LIB_SRCS
   KoColorSet.cpp
   KoResource.cpp
)

kde4_automoc(${koresources_LIB_SRCS})

kde4_add_library(koresources SHARED ${koresources_LIB_SRCS})

target_link_libraries(koresources  ${KDE4_KDECORE_LIBS} pigment)

set_target_properties(koresources PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koresources  DESTINATION ${LIB_INSTALL_DIR})

