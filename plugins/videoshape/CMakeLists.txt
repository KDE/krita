include_directories(
    ${FLAKE_INCLUDES}
    ${PHONON_INCLUDES}
)

# The next line is a workaround for a issue wherein FindPhononInternal.cmake would change the value of CMAKE_AUTOMOC
# and cause build error "CMake Error: Attempt to add a custom rule to output ...."
set(CMAKE_AUTOMOC_TEMP ${CMAKE_AUTOMOC})
find_package(Phonon NO_MODULE)
set(CMAKE_AUTOMOC ${CMAKE_AUTOMOC_TEMP})

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

target_link_libraries(calligra_shape_video flake
    ${KDE4_KFILE_LIBRARY}
    ${PHONON_LIBS}
)

install(TARGETS calligra_shape_video DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_video.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
