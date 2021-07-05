if(NOT APPLE)
add_subdirectory(tests)
endif()

set(KRITA_STORYBOARDDOCKER_SOURCES
    CommentModel.cpp
    StoryboardDocker.cpp
    StoryboardDockerDock.cpp
    CommentDelegate.cpp
    StoryboardModel.cpp
    StoryboardDelegate.cpp
    StoryboardView.cpp
    KisAsyncStoryboardThumbnailRenderer.cpp
    KisStoryboardThumbnailRenderScheduler.cpp
    DlgExportStoryboard.cpp
    KisAddRemoveStoryboardCommand.cpp
    )

set(KRITA_STORYBOARDDOCKER_PART_HEADERS
    CommentModel.h
    StoryboardDocker.h
    StoryboardDockerDock.h
    CommentDelegate.h
    StoryboardModel.h
    StoryboardDelegate.h
    StoryboardView.h
    KisAsyncStoryboardThumbnailRenderer.h
    KisStoryboardThumbnailRenderScheduler.h
    DlgExportStoryboard.h
    KisAddRemoveStoryboardCommand.h
)

ki18n_wrap_ui(KRITA_STORYBOARDDOCKER_SOURCES
    wdgstoryboarddock.ui wdgarrangemenu.ui wdgcommentmenu.ui wdgexportstoryboard.ui
)

add_library(kritastoryboarddocker MODULE ${KRITA_STORYBOARDDOCKER_SOURCES}
                                         ${KRITA_STORYBOARDDOCKER_PART_HEADERS}
)

generate_export_header(kritastoryboarddocker BASE_NAME kritastoryboarddocker)

target_link_libraries(kritastoryboarddocker kritaui)
install(TARGETS kritastoryboarddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
