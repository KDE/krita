set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/plugins/textshape 
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
    ${CMAKE_SOURCE_DIR}/plugins/textshape/dialogs
    ${CMAKE_BINARY_DIR}/plugins/textshape/dialogs
    ${CMAKE_BINARY_DIR}/plugins/textshape
    ${KOTEXT_INCLUDES} )

if(NOT NO_PIGMENT)
    include_directories(${PIGMENT_INCLUDES})
endif(NOT NO_PIGMENT)


########### next target ###############

set(TestDocumentLayout_test_SRCS
    TestDocumentLayout.cpp
    TestLists.cpp
    ../Layout.cpp
    ../TextShape.cpp
    ../ListItemsHelper.cpp
    ../TableLayout.cpp
    ../TableLayoutData.cpp
)

kde4_add_unit_test(TestDocumentLayout TESTNAME shapes-text-TestDocumentLayout ${TestDocumentLayout_test_SRCS})
if(NOT NO_PIGMENT)
    set(EXTRA_LIBS kocolorwidgets)
endif(NOT NO_PIGMENT)

target_link_libraries(TestDocumentLayout ${EXTRA_LIBS} komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangeListCommand_test_SRCS
    TestChangeListCommand.cpp
    ../commands/ChangeListCommand.cpp
    ../commands/ChangeListLevelCommand.cpp
    ../commands/ListItemNumberingCommand.cpp
    ../commands/TextCommandBase.cpp
    ../commands/TextPasteCommand.cpp
    ../commands/TextCutCommand.cpp
    ../commands/DeleteCommand.cpp
    ../commands/ShowChangesCommand.cpp
    ../TextTool.cpp
    ../ChangeTracker.cpp
    ../dialogs/FontDia.cpp
    ../dialogs/InsertCharacter.cpp
    ../dialogs/StyleManagerDialog.cpp
    ../dialogs/ParagraphSettingsDialog.cpp
    ../dialogs/SimpleStyleWidget.cpp
    ../dialogs/StylesWidget.cpp
    ../dialogs/FontTab.cpp
    ../dialogs/CharacterHighlighting.cpp
    ../dialogs/FontDecorations.cpp
    ../dialogs/FontLayoutTab.cpp
    ../dialogs/LanguageTab.cpp
    ../dialogs/StyleManager.cpp
    ../dialogs/ParagraphIndentSpacing.cpp
    ../dialogs/ParagraphLayout.cpp
    ../dialogs/ParagraphDecorations.cpp
    ../dialogs/ParagraphBulletsNumbers.cpp
    ../dialogs/ListsSpinBox.cpp
    ../dialogs/StylesModel.cpp
    ../dialogs/ParagraphGeneral.cpp
    ../dialogs/CharacterGeneral.cpp
    ../dialogs/NewStyleWidget.cpp
    ../dialogs/FormattingPreview.cpp
    ../dialogs/StyleManagerWelcome.cpp
    ../dialogs/TableDialog.cpp
    ../ListItemsHelper.cpp
    ../TextShape.cpp
    ../Layout.cpp
    ../TableLayout.cpp
    ../TableLayoutData.cpp
)


kde4_add_unit_test(TestChangeListCommand TESTNAME shapes-text-TestChangeListCommand ${TestChangeListCommand_test_SRCS})
target_link_libraries(TestChangeListCommand ${EXTRA_LIBS} komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangesDatabase_test_SRCS
    TestChangesDatabase.cpp
    ../TextChanges.cpp
    ../TextChange.cpp
)
kde4_add_unit_test(TestChangesDatabase TESTNAME shapes-text-TestChangesDatabase ${TestChangesDatabase_test_SRCS})
target_link_libraries(TestChangesDatabase ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestStylesModel_test_SRCS
    TestStylesModel.cpp
    ../dialogs/StylesModel.cpp
)
kde4_add_unit_test(TestStylesModel TESTNAME shapes-text-TestStylesModel ${TestStylesModel_test_SRCS})
target_link_libraries(TestStylesModel kotext ${EXTRA_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTableLayout_test_SRCS
    TestTableLayout.cpp
    ../TableLayout.cpp
    ../TableLayoutData.cpp
    ../Layout.cpp
    ../TextShape.cpp
    ../ListItemsHelper.cpp
)
kde4_add_unit_test(TestTableLayout TESTNAME shapes-text-TestTableLayout ${TestTableLayout_test_SRCS})
target_link_libraries(TestTableLayout kotext komain ${QT_QTTEST_LIBRARY})
