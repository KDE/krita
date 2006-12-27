
# if(KUNITTEST_FOUND)
#     add_subdirectory(tests)
# endif(KUNITTEST_FOUND)



########### next target ###############

if(OPENEXR_FOUND)

set(kritargbf16hdr_LIB_SRCS kis_rgb_f16_hdr_colorspace.cc )

kde4_automoc(${kritargbf16hdr_LIB_SRCS})

kde4_add_library(kritargbf16hdr SHARED ${kritargbf16hdr_LIB_SRCS})

target_link_libraries(kritargbf16hdr  ${KDE4_KDECORE_LIBS} pigment )

set_target_properties(kritargbf16hdr PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritargbf16hdr  DESTINATION ${LIB_INSTALL_DIR})

endif(OPENEXR_FOUND)

########### next target ###############

set(kritargbf32hdr_LIB_SRCS kis_rgb_f32_hdr_colorspace.cc )

kde4_automoc(${kritargbf32hdr_LIB_SRCS})

kde4_add_library(kritargbf32hdr SHARED ${kritargbf32hdr_LIB_SRCS})

target_link_libraries(kritargbf32hdr  ${KDE4_KDECORE_LIBS} pigment )

set_target_properties(kritargbf32hdr PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritargbf32hdr  DESTINATION ${LIB_INSTALL_DIR})

########### next target ###############

set(kritargbfloathdrplugin_PART_SRCS rgb_float_hdr.cc )

kde4_automoc(${kritargbfloathdrplugin_PART_SRCS})

kde4_add_plugin(kritargbfloathdrplugin ${kritargbfloathdrplugin_PART_SRCS})



target_link_libraries(kritargbfloathdrplugin  ${KDE4_KDECORE_LIBS} kritargbf16hdr kritargbf32hdr  pigment)

install(TARGETS kritargbfloathdrplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritargbfloathdrplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
