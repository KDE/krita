project(changecase)

include_directories(   ${KOTEXT_INCLUDES} )

set(changecase_SRCS
    ChangecasePlugin.cpp
    Changecase.cpp
    ChangecaseFactory.cpp
)

add_library(calligra_textediting_changecase MODULE ${changecase_SRCS})

target_link_libraries(calligra_textediting_changecase kotext)

install(TARGETS calligra_textediting_changecase DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_textediting_changecase.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
