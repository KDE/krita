set(kritabuginfo_SOURCES
    buginfo.cpp
    dlg_buginfo.cpp
    DlgSysInfo.cpp
    DlgKritaLog.cpp
)

ki18n_wrap_ui(kritabuginfo_SOURCES wdg_buginfo.ui )

add_library(kritabuginfo MODULE ${kritabuginfo_SOURCES})

target_link_libraries(kritabuginfo kritaui)

install(TARGETS kritabuginfo  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES  buginfo.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
