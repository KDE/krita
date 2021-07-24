if (WebP_FOUND)
    set(kritawebpexport_SOURCES
        dlg_webp_export.cpp
        kis_webp_export.cpp
    )
    ki18n_wrap_ui(kritawebpexport_SOURCES dlg_webp_export.ui)

    add_library(kritawebpexport MODULE ${kritawebpexport_SOURCES})
    target_include_directories(kritawebpexport PRIVATE ${WebP_INCLUDE_DIRS})
    target_link_libraries(kritawebpexport kritaui kritalibkra kritaimpex WebP::webp)

    install(TARGETS kritawebpexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


    set(kritawebpimport_SOURCES
        dlg_webp_import.cpp
        kis_webp_import.cpp
    )
    ki18n_wrap_ui(kritawebpimport_SOURCES dlg_webp_import.ui)

    add_library(kritawebpimport MODULE ${kritawebpimport_SOURCES})
    target_include_directories(kritawebpimport PRIVATE ${WebP_INCLUDE_DIRS})
    target_link_libraries(kritawebpimport kritaui WebP::webp)

    install(TARGETS kritawebpimport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
    install(PROGRAMS krita_webp.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
endif()
