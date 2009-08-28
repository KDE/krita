include_directories(${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/pagelayout
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/libs/widgets/input
    ${KOTEXT_INCLUDES})

########### next target ###############

set(kopagelayout_LIB_SRCS
    KoPageLayout.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
)


kde4_add_ui_files( kopagelayout_LIB_SRCS
    KoPageLayoutWidget.ui
)

kde4_add_library(kopagelayout SHARED ${kopagelayout_LIB_SRCS})

target_link_libraries(kopagelayout kotext koinput)

set_target_properties(kopagelayout PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kopagelayout ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES
    KoPageLayout.h
    KoPageLayoutDialog.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    ../kowidgets_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
