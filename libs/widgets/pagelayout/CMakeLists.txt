include_directories(${KDE4_INCLUDES} 
    ${CMAKE_SOURCE_DIR}/libs/widgets/pagelayout
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/libs/widgets/input
    ${CMAKE_SOURCE_DIR}/libs/store
    ${CMAKE_SOURCE_DIR}/libs/kotext
    ${CMAKE_SOURCE_DIR}/libs/odf
    ${CMAKE_SOURCE_DIR}/libs/kobase
    ${CMAKE_SOURCE_DIR}/libs/flake
)

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

target_link_libraries(kopagelayout kotext koodf kostore kobase koinput flake
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KFILE_LIBRARY}
)

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
