include_directories(
    ${KOMAIN_INCLUDES}
)

set(pivottables_SRCS
   pivot.cpp
   pivotfilters.cpp
   pivotmain.cpp
   pivotoptions.cpp
   pivotplugin.cpp)

ki18n_wrap_ui(pivottables_SRCS
   pivot.ui
   pivotfilters.ui
   pivotmain.ui
   pivotoptions.ui)

add_library(sheetspivottables MODULE ${pivottables_SRCS})
kcoreaddons_desktop_to_json(sheetspivottables sheetspivottables.desktop)

target_link_libraries(sheetspivottables calligrasheetscommon)

install(TARGETS sheetspivottables DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  pivottables.rc  DESTINATION ${DATA_INSTALL_DIR}/sheets/viewplugins)


