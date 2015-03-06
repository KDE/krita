project(textvariables)

include_directories(
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${KOTEXT_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
)

set(textvariables_SRCS
    VariablesPlugin.cpp
    UserVariable.cpp
    UserVariableFactory.cpp
    UserVariableOptionsWidget.cpp
    DateVariable.cpp
    DateVariableFactory.cpp
    FixedDateFormat.cpp
    PageVariable.cpp
    PageVariableFactory.cpp
    InfoVariable.cpp
    InfoVariableFactory.cpp
    ChapterVariable.cpp
    ChapterVariableFactory.cpp
)


ki18n_wrap_ui( textvariables_SRCS
    FixedDateFormat.ui
)

add_library(calligra_textinlineobject_variables MODULE ${textvariables_SRCS})

target_link_libraries(calligra_textinlineobject_variables kotext kotextlayout)

install(TARGETS calligra_textinlineobject_variables DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_textinlineobject_variables.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
