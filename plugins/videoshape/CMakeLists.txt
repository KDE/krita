include_directories( ${PHONON_INCLUDES} )

# The next line is a workaround for a issue wherein FindPhononInternal.cmake would change the value of CMAKE_AUTOMOC
# and cause build error "CMake Error: Attempt to add a custom rule to output ...."
set(CMAKE_AUTOMOC_TEMP ${CMAKE_AUTOMOC})
find_package(Phonon CONFIG)
set(CMAKE_AUTOMOC ${CMAKE_AUTOMOC_TEMP})

if(PHONON_FOUND_EXPERIMENTAL)
   set(SHOULD_BUILD_THUMBNAIL TRUE)
   add_definitions( -DSHOULD_BUILD_THUMBNAIL )
else(PHONON_FOUND_EXPERIMENTAL)
    set(SHOULD_BUILD_THUMBNAIL FALSE)
endif(PHONON_FOUND_EXPERIMENTAL)


########### Flake Plugin library ###############

SET (videoshape_LIB_SRCS
    VideoData.cpp
    VideoCollection.cpp
    VideoShape.cpp
    VideoShapeFactory.cpp
    VideoEventAction.cpp
    Plugin.cpp
    VideoShapeConfigWidget.cpp
    FullScreenPlayer.cpp
    VideoTool.cpp
    VideoToolFactory.cpp
    ChangeVideoCommand.cpp
    SelectVideoWidget.cpp
)

if(SHOULD_BUILD_THUMBNAIL)
    set(videoshape_LIB_SRCS ${videoshape_LIB_SRCS} VideoThumbnailer.cpp)
endif(SHOULD_BUILD_THUMBNAIL)

kde4_add_ui_files(videoshape_LIB_SRCS forms/VideoToolWidget.ui)
kde4_add_plugin(videoshape ${videoshape_LIB_SRCS})

target_link_libraries(videoshape komain
    ${KDE4_KFILE_LIBRARY}
    ${PHONON_LIBS}
)

install(TARGETS videoshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES videoshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
