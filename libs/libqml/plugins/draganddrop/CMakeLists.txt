include(GNUInstallDirs)

set(declarativedragdrop_SRCS
    draganddropplugin.cpp
    DeclarativeDragArea.cpp
    DeclarativeDragDropEvent.cpp
    DeclarativeDropArea.cpp
    DeclarativeMimeData.cpp
    )

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        
)

add_library(draganddropplugin SHARED ${declarativedragdrop_SRCS})
target_link_libraries(draganddropplugin Qt5::Core Qt5::Gui Qt5::Quick kritawidgetutils)

install(TARGETS draganddropplugin DESTINATION ${QML_INSTALL_DIR}/org/krita/draganddrop)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/krita/draganddrop)
