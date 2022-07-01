add_subdirectory(tests)

set(KRITA_STORYBOARDDOCKER_SOURCES
    CommentModel.cpp
    StoryboardUtils.cpp
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

ki18n_wrap_ui(KRITA_STORYBOARDDOCKER_SOURCES
    wdgstoryboarddock.ui wdgarrangemenu.ui wdgcommentmenu.ui wdgexportstoryboard.ui
)

kis_add_library(kritastoryboarddocker_static STATIC ${KRITA_STORYBOARDDOCKER_SOURCES}
)

generate_export_header(kritastoryboarddocker_static BASE_NAME kritastoryboarddocker)

target_link_libraries(kritastoryboarddocker_static kritaui)

kis_add_library(kritastoryboarddocker MODULE StoryboardDocker.cpp)

target_link_libraries(kritastoryboarddocker kritastoryboarddocker_static)

install(TARGETS kritastoryboarddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
