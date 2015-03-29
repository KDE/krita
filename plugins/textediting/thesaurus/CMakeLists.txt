


include_directories(  ${KOTEXT_INCLUDES} )

########### next target ###############

set(thesaurustool_SRCS
    Thesaurus.cpp
    ThesaurusPlugin.cpp
    ThesaurusFactory.cpp
)


add_library(calligra_textediting_thesaurus MODULE ${thesaurustool_SRCS})
kcoreaddons_desktop_to_json(calligra_textediting_thesaurus calligra_textediting_thesaurus.desktop)

target_link_libraries(calligra_textediting_thesaurus kotext)

install(TARGETS calligra_textediting_thesaurus  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  thesaurus.txt  DESTINATION ${DATA_INSTALL_DIR}/calligra/thesaurus)

