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

target_link_libraries(calligra_textediting_autocorrect kowidgets)

install(TARGETS calligra_textediting_autocorrect DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_textediting_autocorrect.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
