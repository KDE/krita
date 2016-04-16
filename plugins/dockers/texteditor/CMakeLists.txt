set(KRITA_texteditor_SOURCES texteditor.cpp texteditor_dock.cpp WdgTextEditor.cpp)

ki18n_wrap_ui(KRITA_texteditor_SOURCES WdgTextEditor.ui)

add_library(kritatexteditor MODULE ${KRITA_texteditor_SOURCES})

target_link_libraries(kritatexteditor kritaui)

install(TARGETS kritatexteditor  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
