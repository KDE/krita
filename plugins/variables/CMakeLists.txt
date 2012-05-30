project(textvariables)

include_directories(
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${KOTEXT_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
    ${KOMAIN_INCLUDES} )

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


kde4_add_ui_files( textvariables_SRCS
    FixedDateFormat.ui
)

kde4_add_plugin(textvariables ${textvariables_SRCS})

target_link_libraries(textvariables kotext textlayout)

install(TARGETS textvariables DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES textvariables.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
