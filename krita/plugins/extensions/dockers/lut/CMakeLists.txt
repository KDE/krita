include_directories( ${KRITA_INCLUDES} ${OCIO_INCLUDE_DIRECTORY})
add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(KRITA_LUTDOCKER_PART_SRCS lutdocker.cpp lutdocker_dock.cpp ocio_display_filter.cpp )

kde4_add_ui_files(KRITA_LUTDOCKER_PART_SRCS
    wdglut.ui
)

kde4_add_plugin(kritalutdocker ${KRITA_LUTDOCKER_PART_SRCS})
target_link_libraries(kritalutdocker kritaui ${OCIO_LIBRARIES})

if(HAVE_OPENGL)
    if(HAVE_GLEW)
        target_link_libraries(kritalutdocker ${GLEW_LIBRARIES})
    endif(HAVE_GLEW)
endif(HAVE_OPENGL)


install(TARGETS kritalutdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_lutdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

