


if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############

set(kritagrayscale_LIB_SRCS kis_gray_colorspace.cc )


kde4_add_library(kritagrayscale SHARED ${kritagrayscale_LIB_SRCS})

target_link_libraries(kritagrayscale pigmentcms )

set_target_properties(kritagrayscale PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritagrayscale  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kritagrayplugin_PART_SRCS gray_plugin.cc )


kde4_add_plugin(kritagrayplugin ${kritagrayplugin_PART_SRCS})



target_link_libraries(kritagrayplugin kritagrayscale pigmentcms )

install(TARGETS kritagrayplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritagrayplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

