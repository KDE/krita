

add_subdirectory( templates )

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############

set(kritagrayscale_LIB_SRCS kis_gray_colorspace.cc )

kde4_automoc(${kritagrayscale_LIB_SRCS})

kde4_add_library(kritagrayscale SHARED ${kritagrayscale_LIB_SRCS})

target_link_libraries(kritagrayscale  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(kritagrayscale PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritagrayscale  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kritagrayplugin_PART_SRCS gray_plugin.cc )

kde4_automoc(${kritagrayplugin_PART_SRCS})

kde4_add_plugin(kritagrayplugin ${kritagrayplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritagrayplugin )

target_link_libraries(kritagrayplugin  ${KDE4_KDECORE_LIBS} kritagrayscale kritacolor )

install(TARGETS kritagrayplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritagrayplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

