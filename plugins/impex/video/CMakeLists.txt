macro_optional_find_package(AvTranscoder)
macro_log_feature(AvTranscoder_FOUND "AvTranscoder" "AvTranscoder Library" "https://github.com/avTranscoder/avTranscoder" FALSE "0.9.5" "Required for converting animations to video files")

if (AvTranscoder_FOUND)
 message("Found AvTranscoder")
else (AvTranscoder_FOUND)
 message("Can't find AvTranscoder. Video export plugin will be skipped!")
 return()
endif (AvTranscoder_FOUND)

## If the necessary libraries are present, continue with building the
## plugin

add_subdirectory(tests)

include_directories(SYSTEM
  ${AvTranscoder_INCLUDE_DIR}
)

# export

set(kritavideoexport_SOURCES
    kis_video_export.cpp
    video_saver.cpp
)

add_library(kritavideoexport MODULE ${kritavideoexport_SOURCES})
generate_export_header(kritavideoexport BASE_NAME kritavideoexport)

target_link_libraries(kritavideoexport kritaui ${AvTranscoder_LIBRARIES})

install(TARGETS kritavideoexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
