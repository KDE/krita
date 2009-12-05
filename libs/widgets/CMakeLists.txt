add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES})

set(kowidgets_LIB_SRCS
    KoAspectButton.cpp
    KoCsvImportDialog.cpp
    KoImageResource.cpp
    KoPageLayout.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
    KoPositionSelector.cpp
    KoSliderCombo.cpp
    KoUnitDoubleSpinBox.cpp
    KoZoomAction.cpp
    KoZoomController.cpp
    KoZoomInput.cpp
    KoZoomHandler.cpp
    KoZoomMode.cpp
    KoDpi.cpp
)

kde4_add_ui_files( kowidgets_LIB_SRCS
    KoCsvImportDialog.ui
    KoPageLayoutWidget.ui
)

kde4_add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})

target_link_libraries(kowidgets kotext)

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoGenericRegistryModel.h
    KoPageLayoutDialog.h
    KoPageLayout.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    KoPositionSelector.h
    kowidgets_export.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomInput.h
    KoDpi.h
    KoZoomHandler.h
    KoZoomMode.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
