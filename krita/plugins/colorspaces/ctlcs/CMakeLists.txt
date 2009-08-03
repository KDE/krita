include_directories(${OPENCTL_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(${OPENCTL_DEFINITIONS})

add_subdirectory(ctlmodules)
add_subdirectory(ctlprofiles)
add_subdirectory(ctlcolorspaces)
add_subdirectory(ctlcompositeops)

# This is temporary, when we are close to koffice release (and when OpenCTL 0.9.10 is released), make this a hard requirement
configure_file(config-openctl910.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openctl910.h )

if( HAVE_OPENCTL_910 )
  set( SRCS_910
      KoCtlCompositeOp.cpp
      KoCtlTemplatesRegistry.cpp
  )
endif( HAVE_OPENCTL_910 )

set( CTLCS_SRCS
    KoCtlBuffer.cpp
    KoCtlChannel.cpp
    KoCtlColorProfile.cpp
    KoCtlColorConversionTransformation.cpp
    KoCtlColorSpace.cpp
    KoCtlColorSpaceFactory.cpp
    KoCtlColorSpaceInfo.cpp
    KoCtlParser.cpp
    ctl_cs_plugin.cpp
    ${SRCS_910}
)


kde4_add_plugin(krita_ctlcs_plugin ${CTLCS_SRCS})

target_link_libraries(krita_ctlcs_plugin pigmentcms ${KDE4_KPARTS_LIBS} ${OPENEXR_LIBRARIES} ${OPENCTL_LIBRARIES})

install(TARGETS krita_ctlcs_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES  krita_ctlcs_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


add_subdirectory(tests)
