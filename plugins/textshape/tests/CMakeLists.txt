set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/plugins/textshape ${CMAKE_BINARY_DIR}/plugins/textshape ${KOGUIUTILS_INCLUDES} ${KOTEXT_INCLUDES} )

########### next target ###############

set(TestDocumentLayout_test_SRCS
    TestDocumentLayout.cpp
    TestLists.cpp
    ../Layout.cpp
    ../TextShape.cpp
    ../ListItemsHelper.cpp
)

kde4_add_unit_test(TestDocumentLayout TESTNAME shapes-text-TestDocumentLayout ${TestDocumentLayout_test_SRCS})
target_link_libraries(TestDocumentLayout kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangeListCommand_test_SRCS
    TestChangeListCommand.cpp
    ../commands/ChangeListCommand.cpp
    ../commands/ChangeListLevelCommand.cpp
    ../commands/ListItemNumberingCommand.cpp
    ../commands/TextCommandBase.cpp
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
    ../ListItemsHelper.cpp
    ../TextShape.cpp
    ../Layout.cpp    
)


kde4_add_unit_test(TestChangeListCommand TESTNAME shapes-text-TestChangeListCommand ${TestChangeListCommand_test_SRCS})
target_link_libraries(TestChangeListCommand koguiutils kotext ${QT_QTTEST_LIBRARY}  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBS})

########### next target ###############

set(TestChangesDatabase_test_SRCS
    TestChangesDatabase.cpp
    ../TextChanges.cpp
    ../TextChange.cpp
)
kde4_add_unit_test(TestChangesDatabase TESTNAME shapes-text-TestChangesDatabase ${TestChangesDatabase_test_SRCS})
target_link_libraries(TestChangesDatabase ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY})

########### next target ###############

set(TestStylesModel_test_SRCS
    TestStylesModel.cpp
    ../dialogs/StylesModel.cpp
)
kde4_add_unit_test(TestStylesModel TESTNAME shapes-text-TestStylesModel ${TestStylesModel_test_SRCS})
target_link_libraries(TestStylesModel kotext ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})

