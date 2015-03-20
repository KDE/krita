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

add_library(koreport_webplugin MODULE ${koreport_webplugin_LIB_SRCS})

set (QT_USE_QTWEBKIT TRUE)

target_link_libraries(koreport_webplugin
    ${KOREPORT_LIBS}
    ${KOPROPERTY_LIBS}
    
    Qt5::Core
    Qt5::Gui
    Qt5::WebKitWidgets
)

########### install files ###############

install(TARGETS koreport_webplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_webplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
