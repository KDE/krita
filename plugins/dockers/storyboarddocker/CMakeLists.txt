#add_subdirectory(tests)

set(KRITA_STORYBOARDDOCKER_SOURCES
    commentModel.cpp
    storyboarddocker.cpp
    storyboarddocker_dock.cpp
    commentDelegate.cpp
    storyboardModel.cpp
    storyboardItem.cpp
    storyboardDelegate.cpp
    storyboardView.cpp
    )

set(KRITA_STORYBOARDDOCKER_PART_HEADERS
    commentModel.h
    storyboarddocker.h
    storyboarddocker_dock.h
    commentDelegate.h
    storyboardModel.h
    storyboardItem.h
    storyboardDelegate.h
    storyboardView.h
)

ki18n_wrap_ui(KRITA_STORYBOARDDOCKER_SOURCES
    wdgstoryboarddock.ui wdgarrangemenu.ui wdgcommentmenu.ui
)

add_library(kritastoryboarddocker MODULE ${KRITA_STORYBOARDDOCKER_SOURCES}
                                         ${KRITA_STORYBOARDDOCKER_PART_HEADERS}
)

target_link_libraries(kritastoryboarddocker kritaui)
install(TARGETS kritastoryboarddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
