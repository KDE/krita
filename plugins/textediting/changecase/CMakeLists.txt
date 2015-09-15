project(changecase)

add_definitions(-DTRANSLATION_DOMAIN=\"calligra_textediting_changecase\")

include_directories( ${KOWIDGETS_INCLUDES} ${KOTEXT_INCLUDES} )

set(changecase_SRCS
    ChangecasePlugin.cpp
    Changecase.cpp
    ChangecaseFactory.cpp
)

add_library(calligra_textediting_changecase MODULE ${changecase_SRCS})
kcoreaddons_desktop_to_json(calligra_textediting_changecase calligra_textediting_changecase.desktop)

target_link_libraries(calligra_textediting_changecase kotext kowidgets KF5::KDELibs4Support)

install(TARGETS calligra_textediting_changecase DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
