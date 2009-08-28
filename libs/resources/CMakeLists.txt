include_directories( ${KORESOURCES_INCLUDES} )

########### next target ###############

set(koresources_LIB_SRCS
   KoColorSet.cpp
   KoPattern.cpp
   KoAbstractGradient.cpp
   KoStopGradient.cpp
   KoSegmentGradient.cc
   KoResource.cpp
   KoResourceServerProvider.cpp
   KoResourceServerAdapter.cpp
)

kde4_add_library(koresources SHARED ${koresources_LIB_SRCS})

target_link_libraries(koresources pigmentcms ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})
target_link_libraries(koresources LINK_INTERFACE_LIBRARIES pigmentcms)

set_target_properties(koresources PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS koresources  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES 
         koresource_export.h
         KoAbstractGradient.h
         KoColorSet.h
         KoPattern.h
         KoResource.h
         KoResourceServer.h
         KoResourceServerObserver.h
         KoResourceServerProvider.h
         KoSegmentGradient.h
         KoStopGradient.h
         DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

