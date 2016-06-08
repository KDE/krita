macro_optional_find_package(AvTranscoder)
macro_log_feature(AvTranscoder_FOUND "AvTranscoder" "AvTranscoder Library" "https://github.com/avTranscoder/avTranscoder" FALSE "0.9.5" "Required for converting animations to video files")

if (AvTranscoder_FOUND)
 message("Found AvTranscoder")
else (AvTranscoder_FOUND)
 message("Can't find AvTranscoder. Video export plugin will be skipped!")
 return()
endif (AvTranscoder_FOUND)

# Find package ffmpeg/libav: avtranscoder needs to have the ffmpeg include directories in order to build
find_package(FFmpeg COMPONENTS avcodec avformat avutil swscale avfilter swresample avresample)
if(NOT swresample_FOUND)
    message(SEND_ERROR "libav is not supported as an ffmpeg alternative")
endif()



## If the necessary libraries are present, continue with building the
## plugin

add_subdirectory(tests)

include_directories(SYSTEM
    ${AvTranscoder_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIR}
)

# export

set(kritavideoexport_SOURCES
    kis_video_export.cpp
    video_saver.cpp
    video_export_options_dialog.cpp
    )

ki18n_wrap_ui(kritavideoexport_SOURCES video_export_options_dialog.ui )

add_library(kritavideoexport MODULE ${kritavideoexport_SOURCES})
generate_export_header(kritavideoexport BASE_NAME kritavideoexport)

target_link_libraries(kritavideoexport kritaui ${AvTranscoder_LIBRARIES})

install(TARGETS kritavideoexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
