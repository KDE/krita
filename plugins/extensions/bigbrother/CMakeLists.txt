set(kritabigbrother_SOURCES
        bigbrother.cc
        actionseditor/kis_actions_editor.cpp
        actionseditor/kis_actions_editor_dialog.cpp
        actionseditor/kis_macro_model.cpp )

ki18n_wrap_ui(kritabigbrother_SOURCES
        actionseditor/wdgactionseditor.ui )

add_library(kritabigbrother MODULE ${kritabigbrother_SOURCES})
target_link_libraries(kritabigbrother kritaui)
install(TARGETS kritabigbrother  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  bigbrother.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
