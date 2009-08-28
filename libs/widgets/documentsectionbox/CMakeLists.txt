include_directories(${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR}/libs/widgets)
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kodocumentsectionbox_LIB_SRCS
    KoDocumentSectionDelegate.cpp
    KoDocumentSectionToolTip.cpp
    KoDocumentSectionView.cpp
    ../KoItemToolTip.cpp
)

kde4_add_library(kodocumentsectionbox SHARED ${kodocumentsectionbox_LIB_SRCS})

target_link_libraries(kodocumentsectionbox
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
)

set_target_properties(kodocumentsectionbox PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kodocumentsectionbox ${INSTALL_TARGETS_DEFAULT_ARGS} )
