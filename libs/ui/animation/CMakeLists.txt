include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/qtlockedfile
)

set(kritaanimation_LIB_SRCS
    KisFFMpegWrapper.cpp
    KisVideoSaver.cpp
    KisAnimationRenderingOptions.cpp
    KisAnimationRender.cpp
    KisDlgAnimationRenderer.cpp
    VideoExportOptionsDialog.cpp
    VideoHDRMetadataOptionsDialog.cpp
    KisHDRMetadataOptions.cpp
    KisDlgImportVideoAnimation.cpp
)

ki18n_wrap_ui(kritaanimation_LIB_SRCS
    wdg_animationrenderer.ui
    video_export_options_dialog.ui
    VideoHDRMetadataOptionsDialog.ui
    VideoImportDialog.ui
    
)

add_library(kritaanimation SHARED ${kritaanimation_HEADERS_MOC} ${kritaanimation_LIB_SRCS} )
generate_export_header(kritaanimation BASE_NAME kritaanimation)

target_link_libraries( kritaanimation PUBLIC kritaui )

set_target_properties(kritaanimation PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaanimation  ${INSTALL_TARGETS_DEFAULT_ARGS})

