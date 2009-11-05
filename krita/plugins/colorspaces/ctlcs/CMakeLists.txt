include_directories(${OPENCTL_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(${OPENCTL_DEFINITIONS})

add_subdirectory(ctlmodules)
add_subdirectory(ctlprofiles)
add_subdirectory(ctlcolorspaces)
add_subdirectory(ctlcompositeops)

set( CTLCS_SRCS
    KoCtlAccumulator.cpp
    KoCtlBuffer.cpp
    KoCtlChannel.cpp
    KoCtlMixColorsOp.cpp
    KoCtlColorProfile.cpp
    KoCtlColorConversionTransformation.cpp
    KoCtlColorSpace.cpp
    KoCtlColorSpaceFactory.cpp
    KoCtlColorSpaceInfo.cpp
    KoCtlConvolutionOp.cpp
    KoCtlParser.cpp
    ctl_cs_plugin.cpp
    KoCtlCompositeOp.cpp
    KoCtlTemplatesRegistry.cpp
)


kde4_add_plugin(krita_ctlcs_plugin ${CTLCS_SRCS})

target_link_libraries(krita_ctlcs_plugin pigmentcms ${KDE4_KPARTS_LIBS} ${OPENEXR_LIBRARIES} ${OPENCTL_LIBRARIES})

install(TARGETS krita_ctlcs_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES  krita_ctlcs_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


add_subdirectory(tests)
