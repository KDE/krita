include_directories(
    ${FLAKE_INCLUDES}
    ${PHONON_INCLUDES}
)

if(PHONON_FOUND_EXPERIMENTAL)
   set(SHOULD_BUILD_THUMBNAIL TRUE)
   add_definitions( -DSHOULD_BUILD_THUMBNAIL )
else()
    set(SHOULD_BUILD_THUMBNAIL FALSE)
endif()


########### Flake Plugin library ###############

set (videoshape_LIB_SRCS
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
endif()

ki18n_wrap_ui(videoshape_LIB_SRCS forms/VideoToolWidget.ui)
add_library(calligra_shape_video MODULE ${videoshape_LIB_SRCS})
kcoreaddons_desktop_to_json(calligra_shape_video calligra_shape_video.desktop)

target_link_libraries(calligra_shape_video flake
    ${PHONON_LIBS}
)

install(TARGETS calligra_shape_video DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
