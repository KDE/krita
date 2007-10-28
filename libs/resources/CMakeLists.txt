include_directories( ${KOMAIN_INCLUDES} ${PIGMENT_INCLUDES} )


########### next target ###############

set(koresources_LIB_SRCS
   KoColorSet.cpp
   KoPattern.cpp
   KoAbstractGradient.cpp
   KoStopGradient.cpp
   KoSegmentGradient.cc
   KoResource.cpp
   KoResourceServer.cpp
   KoResourceServerRegistry.cpp
)

kde4_add_library(koresources SHARED ${koresources_LIB_SRCS})

target_link_libraries(koresources  ${KDE4_KDECORE_LIBS} pigmentcms)

set_target_properties(koresources PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koresources  DESTINATION ${LIB_INSTALL_DIR})

