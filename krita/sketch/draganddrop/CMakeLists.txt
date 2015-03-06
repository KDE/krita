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
        ${KDE4_INCLUDES}
)

qt4_automoc(${declarativedragdrop_SRCS})


add_library(draganddropplugin SHARED ${declarativedragdrop_SRCS})
target_link_libraries(draganddropplugin Qt5::Core Qt5::Gui Qt5::Declarative)

install(TARGETS draganddropplugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/draganddrop)

install(FILES qmldir DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/draganddrop)
