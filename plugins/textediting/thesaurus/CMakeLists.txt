


include_directories( ${KDE4_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} )

########### next target ###############

set(thesaurustool_SRCS
    Thesaurus.cpp
    ThesaurusPlugin.cpp
    ThesaurusFactory.cpp
)


kde4_add_plugin(calligra_textediting_thesaurus ${thesaurustool_SRCS})

target_link_libraries(calligra_textediting_thesaurus kotext)

install(TARGETS calligra_textediting_thesaurus  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  thesaurustool.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  thesaurus.txt  DESTINATION ${DATA_INSTALL_DIR}/calligra/thesaurus)

