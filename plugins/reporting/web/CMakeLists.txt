include_directories(
    ${KOPROPERTY_INCLUDES}
    ${KOREPORT_INCLUDES}
)

#build a shared library
set(koreport_webplugin_LIB_SRCS
    KoReportDesignerItemWeb.cpp
    KoReportItemWeb.cpp
    KoReportWebPlugin.cpp
)

kde4_add_plugin(koreport_webplugin ${koreport_webplugin_LIB_SRCS})

set (QT_USE_QTWEBKIT TRUE)

target_link_libraries(koreport_webplugin
    ${KDE4_KDECORE_LIBS}
    ${KOREPORT_LIBS}
    ${KOPROPERTY_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTXML_LIBRARY}
)

########### install files ###############

install(TARGETS koreport_webplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_webplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
