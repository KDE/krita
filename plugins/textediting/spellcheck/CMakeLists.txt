project(spellcheck)
add_subdirectory( tests )

include_directories(   ${TEXTLAYOUT_INCLUDES} ${FLAKE_INCLUDES} )

set(spellcheck_SRCS
    SpellCheckPlugin.cpp
    SpellCheck.cpp
    SpellCheckFactory.cpp
    BgSpellCheck.cpp
    SpellCheckMenu.cpp
)

add_library(calligra_textediting_spellcheck MODULE ${spellcheck_SRCS})
kcoreaddons_desktop_to_json(calligra_textediting_spellcheck calligra_textediting_spellcheck.desktop)

target_link_libraries(calligra_textediting_spellcheck kotext kotextlayout KF5::SonnetCore KF5::SonnetUi)

install(TARGETS calligra_textediting_spellcheck DESTINATION ${PLUGIN_INSTALL_DIR})
