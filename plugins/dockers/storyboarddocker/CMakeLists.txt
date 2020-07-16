add_subdirectory(tests)

set(KRITA_STORYBOARDDOCKER_SOURCES
    CommentModel.cpp
    StoryboardDocker.cpp
    StoryboardDockerDock.cpp
    CommentDelegate.cpp
    StoryboardModel.cpp
    StoryboardItem.cpp
    StoryboardDelegate.cpp
    StoryboardView.cpp
    )

set(KRITA_STORYBOARDDOCKER_PART_HEADERS
    CommentModel.h
    StoryboardDocker.h
    StoryboardDockerDock.h
    CommentDelegate.h
    StoryboardModel.h
    StoryboardItem.h
    StoryboardDelegate.h
    StoryboardView.h
)

ki18n_wrap_ui(KRITA_STORYBOARDDOCKER_SOURCES
    wdgstoryboarddock.ui wdgarrangemenu.ui wdgcommentmenu.ui
)

add_library(kritastoryboarddocker MODULE ${KRITA_STORYBOARDDOCKER_SOURCES}
                                         ${KRITA_STORYBOARDDOCKER_PART_HEADERS}
)

target_link_libraries(kritastoryboarddocker kritaui)
install(TARGETS kritastoryboarddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
