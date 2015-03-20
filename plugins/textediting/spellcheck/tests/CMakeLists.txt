set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/plugins/textediting/spellcheck
    ${KOTEXT_INCLUDES})

########### next target ###############

set(TestSpellCheck_SRCS
    TestSpellCheck.cpp
    ../BgSpellCheck.cpp
)

add_executable(TestSpellCheckNAME textedit-TestSpellCheck ${TestSpellCheck_SRCS})
add_test(TestSpellCheck TestSpellCheck)
ecm_mark_as_test(TestSpellCheck)

target_link_libraries(TestSpellCheck kotext Qt5::Test)

########### last target ###############
