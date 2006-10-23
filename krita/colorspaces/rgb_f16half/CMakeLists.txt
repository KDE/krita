

include_directories( ${OPENEXR_INCLUDE_DIR})
if(KUNITTEST_FOUND)
    MESSAGE(STATUS "Need to port tests program")
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)


########### next target ###############

set(krita_rgb_f16half_LIB_SRCS kis_rgb_f16half_colorspace.cc )

kde4_automoc(${krita_rgb_f16half_LIB_SRCS})

kde4_add_library(krita_rgb_f16half SHARED ${krita_rgb_f16half_LIB_SRCS})

target_link_libraries(krita_rgb_f16half  ${KDE4_KDECORE_LIBS} kritacolor ${OPENEXR_LIBRARIES})

set_target_properties(krita_rgb_f16half PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krita_rgb_f16half  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(krita_rgb_f16half_plugin_PART_SRCS rgb_f16half_plugin.cc )

kde4_automoc(${krita_rgb_f16half_plugin_PART_SRCS})

kde4_add_plugin(krita_rgb_f16half_plugin ${krita_rgb_f16half_plugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krita_rgb_f16half_plugin )

target_link_libraries(krita_rgb_f16half_plugin  ${KDE4_KDECORE_LIBS} krita_rgb_f16half kritacolor  ${OPENEXR_LIBRARIES})

install(TARGETS krita_rgb_f16half_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_rgb_f16half_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




