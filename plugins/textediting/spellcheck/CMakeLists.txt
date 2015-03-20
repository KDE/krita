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

target_link_libraries(calligra_textediting_spellcheck kotext kotextlayout)

install(TARGETS calligra_textediting_spellcheck DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_textediting_spellcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
