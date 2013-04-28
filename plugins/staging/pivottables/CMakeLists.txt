
set(pivottables_SRCS
   pivot.cpp
   pivotfilters.cpp
   pivotmain.cpp
   pivotoptions.cpp
   pivotplugin.cpp)

kde4_add_ui_files(pivottables_SRCS
   pivot.ui
   pivotfilters.ui
   pivotmain.ui
   pivotoptions.ui)

kde4_add_plugin(sheetspivottables ${pivottables_SRCS})

target_link_libraries(sheetspivottables calligrasheetscommon)

install(TARGETS sheetspivottables DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  pivottables.rc  DESTINATION ${DATA_INSTALL_DIR}/sheets/kpartplugins)

