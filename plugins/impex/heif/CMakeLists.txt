add_subdirectory(tests)

set(kritaheifimport_SOURCES
    HeifImport.cpp
    HeifError.cpp
)

if(HAVE_XSIMD)
    ko_compile_for_all_implementations(__per_arch_heif kis_heif_import_tools.cpp)

    message("Following objects are generated from the per-arch lib")
    foreach(_obj IN LISTS __per_arch_heif)
        message("    * ${_obj}")
    endforeach()
else()
    set(__per_arch_heif kis_heif_import_tools.cpp)
endif()

kis_add_library(kritaheifimport MODULE ${kritaheifimport_SOURCES} ${__per_arch_heif})

target_link_libraries(kritaheifimport kritaui kritalibkra HEIF::heif)

install(TARGETS kritaheifimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaheifexport_SOURCES
    HeifExport.cpp
    HeifError.cpp
)

ki18n_wrap_ui(kritaheifexport_SOURCES WdgHeifExport.ui)


kis_add_library(kritaheifexport MODULE ${kritaheifexport_SOURCES})

target_link_libraries(kritaheifexport kritaui kritalibkra kritaimpex HEIF::heif)

install(TARGETS kritaheifexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install(FILES krita_heif.desktop DESTINATION ${KDE_INSTALL_APPDIR})
