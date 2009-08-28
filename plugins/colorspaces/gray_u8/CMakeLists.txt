add_subdirectory( templates )

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############

set(kofficegrayau8colorspace_LIB_SRCS kis_gray_colorspace.cc )

kde4_add_library(kofficegrayau8colorspace SHARED ${kofficegrayau8colorspace_LIB_SRCS})

target_link_libraries(kofficegrayau8colorspace pigmentcms)

set_target_properties(kofficegrayau8colorspace PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kofficegrayau8colorspace  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kofficegrayau8plugin_PART_SRCS gray_plugin.cc )


kde4_add_plugin(kofficegrayau8plugin ${kofficegrayau8plugin_PART_SRCS})



target_link_libraries(kofficegrayau8plugin  ${KDE4_KDECORE_LIBS} kofficegrayau8colorspace pigmentcms )

install(TARGETS kofficegrayau8plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kofficegrayaplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

