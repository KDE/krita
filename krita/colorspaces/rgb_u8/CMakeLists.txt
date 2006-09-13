

add_subdirectory( templates )

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)



########### next target ###############

set(kritargb_LIB_SRCS kis_rgb_colorspace.cc kis_rgb_u8_compositeop.cc)

kde4_automoc(${kritargb_LIB_SRCS})

kde4_add_library(kritargb SHARED ${kritargb_LIB_SRCS})

target_link_libraries(kritargb  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(kritargb PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritargb  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kritargbplugin_PART_SRCS rgb_plugin.cc )

kde4_automoc(${kritargbplugin_PART_SRCS})

kde4_add_plugin(kritargbplugin ${kritargbplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritargbplugin )

target_link_libraries(kritargbplugin  ${KDE4_KDECORE_LIBS} kritargb kritacolor )

install(TARGETS kritargbplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritargbplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
