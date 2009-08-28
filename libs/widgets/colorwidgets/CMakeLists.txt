include_directories( ${PIGMENT_INCLUDES} 
                     ${KORESOURCES_INCLUDES} 
                     ${CMAKE_SOURCE_DIR}/libs/widgets/input
                     ${CMAKE_SOURCE_DIR}/libs/widgets
                     ${CMAKE_SOURCE_DIR}/libs/kobase
)

set(kocolorwidgets_LIB_SRCS
    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KoDualColorButton.cpp
    KoEditColorSetDialog.cpp
    KoTriangleColorSelector.cpp
    KoUniColorChooser.cpp
    KoUniColorDialog.cpp
    KoXYColorSelector.cpp
    KoShadowConfigWidget.cpp
    ../KoCheckerBoardPainter.cpp
)

kde4_add_ui_files( kocolorwidgets_LIB_SRCS
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)

kde4_add_library(kocolorwidgets SHARED ${kocolorwidgets_LIB_SRCS})

target_link_libraries(kocolorwidgets koresources koinput ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

target_link_libraries(kocolorwidgets LINK_INTERFACE_LIBRARIES koresources koinput ${KDE4_KDEUI_LIBS})

set_target_properties(kocolorwidgets PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kocolorwidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
    KoColorPatch.h
    KoShadowConfigWidget.h
    KoColorPopupAction.h
    KoColorSetWidget.h
    KoColorSlider.h
    KoDualColorButton.h
    KoEditColorSetDialog.h
    KoTriangleColorSelector.h
    KoUniColorChooser.h
    KoUniColorDialog.h
    KoXYColorSelector.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
