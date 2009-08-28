include_directories( ${KORESOURCES_INCLUDES}  ${CMAKE_SOURCE_DIR}/libs/widgets)

set(koresourcewidget_LIB_SRCS
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    ../KoItemToolTip.cpp
    ../KoCheckerBoardPainter.cpp
)

kde4_add_library(koresourcewidget SHARED ${koresourcewidget_LIB_SRCS})

target_link_libraries(koresourcewidget koresources ${KDE4_KIO_LIBS})

target_link_libraries(koresourcewidget LINK_INTERFACE_LIBRARIES koresources ${KDE4_KDEUI_LIBS})

set_target_properties(koresourcewidget PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koresourcewidget  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
    KoResourceItemChooser.h
    KoResourceSelector.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
