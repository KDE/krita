set(kritabuginfo_SOURCES
    buginfo.cpp
    dlg_buginfo.cpp
    DlgSysInfo.cpp
    DlgKritaLog.cpp
)

if (ANDROID)
    set(kritabuginfo_SOURCES ${kritabuginfo_SOURCES} DlgAndroidLogcatDumper.cpp)
endif()

if (WIN32 OR ANDROID)
    set(kritabuginfo_SOURCES ${kritabuginfo_SOURCES} DlgCrashLog.cpp)
endif()

ki18n_wrap_ui(kritabuginfo_SOURCES wdg_buginfo.ui )

kis_add_library(kritabuginfo MODULE ${kritabuginfo_SOURCES})

target_link_libraries(kritabuginfo kritaui)
if (ANDROID)
    target_link_libraries(kritabuginfo Qt6::AndroidExtras)
endif()

install(TARGETS kritabuginfo  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES  buginfo.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
