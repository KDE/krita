project(autocorrection)

add_subdirectory( data )

include_directories(   ${KOTEXT_INCLUDES} ${KOWIDGETS_INCLUDES} )

set(autocorrect_SRCS
    AutocorrectPlugin.cpp
    Autocorrect.cpp
    AutocorrectFactory.cpp
    AutocorrectConfigDialog.cpp
)

ki18n_wrap_ui( autocorrect_SRCS
    AutocorrectConfig.ui
)

add_library(calligra_textediting_autocorrect MODULE ${autocorrect_SRCS})
kcoreaddons_desktop_to_json(calligra_textediting_autocorrect calligra_textediting_autocorrect.desktop)

target_link_libraries(calligra_textediting_autocorrect kowidgets)

install(TARGETS calligra_textediting_autocorrect DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
