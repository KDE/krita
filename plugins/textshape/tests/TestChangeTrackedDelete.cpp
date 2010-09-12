#include "TestChangeTrackedDelete.h"
#include "tests/MockShapes.h"
#include "../commands/ChangeTrackedDeleteCommand.h"
#include <TextTool.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <QTextDocument>
#include <QTextCursor>
#include <KAction>
#include <KIcon>
#include <KoTextDocumentLayout.h>
#include <KoDeleteChangeMarker.h>
#include <QTextDocumentFragment>
#include <KoInlineTextObjectManager.h>
#include <KoChangeTrackerElement.h>
#include <KoCharacterStyle.h>

TestChangeTrackedDelete::TestChangeTrackedDelete()
{
}

TestChangeTrackedDelete::~TestChangeTrackedDelete()
{
}

void TestChangeTrackedDelete::testDeletePreviousChar()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::PreviousChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(3).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    delCommand->undo();
    QCOMPARE(document->characterAt(3), QChar('l'));
    delete textTool;
}

void TestChangeTrackedDelete::testDeleteNextChar()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(4).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    delCommand->undo();
    QCOMPARE(document->characterAt(4), QChar('o'));
    delete textTool;
}

void TestChangeTrackedDelete::testDeleteSelection()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(2);
    cursor->setPosition(8, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(2).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }
    cursor->setPosition(3);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
    QCOMPARE(deleteData.toPlainText(), QString("llo Wo"));
    delete textTool;
}

void TestChangeTrackedDelete::testPrefixMerge()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(3);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(3).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    cursor->setPosition(4);
    delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }
    cursor->setPosition(4);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
    QCOMPARE(deleteData.toPlainText(), QString("lo"));
    delete textTool;
}

void TestChangeTrackedDelete::testSuffixMerge()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(3);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(3).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    cursor->setPosition(2);
    delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }
    cursor->setPosition(3);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
    QCOMPARE(deleteData.toPlainText(), QString("ll"));
    delete textTool;
}

void TestChangeTrackedDelete::testInterMerge()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(4).unicode(), (ushort)(QChar::ObjectReplacementCharacter));

    cursor->setPosition(2);
    cursor->setPosition(8, QTextCursor::KeepAnchor);
    delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);

    QCOMPARE(document->characterAt(2).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }
    cursor->setPosition(3);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
    QCOMPARE(deleteData.toPlainText(), QString("llo Wor"));
    delete textTool;
}

void TestChangeTrackedDelete::testPartialListItemDelete()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    insertSampleList(document);

    cursor->setPosition(46);
    cursor->setPosition(62, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(46).unicode(), (ushort)(QChar::ObjectReplacementCharacter));

    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }

    cursor->setPosition(47);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();

    QTextDocument deleteDocument;
    QTextCursor deleteCursor(&deleteDocument);

    deleteCursor.insertFragment(deleteData);
    bool listFound = false;

    for (int i=0; i < deleteDocument.characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentList()) {
            listFound = true;
            continue;
        }
    }

    QVERIFY(listFound == true);
    QTextList *deletedList = deleteCursor.currentList();
    bool deletedListStatus = deletedList->format().boolProperty(KoDeleteChangeMarker::DeletedList);
    QVERIFY (deletedListStatus == false);
    bool deletedListItemStatus;
    deletedListItemStatus  = deletedList->item(0).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == false);
    delete textTool;
}

void TestChangeTrackedDelete::testListItemDelete()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    insertSampleList(document);

    cursor->setPosition(46);
    cursor->setPosition(78, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(46).unicode(), (ushort)(QChar::ObjectReplacementCharacter));

    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }

    cursor->setPosition(47);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();

    QTextDocument deleteDocument;
    QTextCursor deleteCursor(&deleteDocument);

    deleteCursor.insertFragment(deleteData);
    bool listFound = false;

    for (int i=0; i < deleteDocument.characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentList()) {
            listFound = true;
            continue;
        }
    }

    QVERIFY(listFound == true);
    QTextList *deletedList = deleteCursor.currentList();
    bool deletedListStatus = deletedList->format().boolProperty(KoDeleteChangeMarker::DeletedList);
    QVERIFY (deletedListStatus == false);
    bool deletedListItemStatus;
    deletedListItemStatus  = deletedList->item(0).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == false);
    deletedListItemStatus  = deletedList->item(1).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    delete textTool;
}

void TestChangeTrackedDelete::testListDelete()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    insertSampleList(document);

    cursor->setPosition(16);
    cursor->setPosition(152, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(16).unicode(), (ushort)(QChar::ObjectReplacementCharacter));

    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }

    cursor->setPosition(17);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();

    QTextDocument deleteDocument;
    QTextCursor deleteCursor(&deleteDocument);

    deleteCursor.insertFragment(deleteData);
    bool listFound = false;

    for (int i=0; i < deleteDocument.characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentList()) {
            listFound = true;
            continue;
        }
    }

    QVERIFY(listFound == true);
    QTextList *deletedList = deleteCursor.currentList();
    bool deletedListStatus = deletedList->format().boolProperty(KoDeleteChangeMarker::DeletedList);
    QVERIFY (deletedListStatus == true);
    bool deletedListItemStatus;
    deletedListItemStatus  = deletedList->item(0).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    deletedListItemStatus  = deletedList->item(1).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    deletedListItemStatus  = deletedList->item(2).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    deletedListItemStatus  = deletedList->item(3).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    deletedListItemStatus  = deletedList->item(4).blockFormat().boolProperty(KoDeleteChangeMarker::DeletedListItem);
    QVERIFY(deletedListItemStatus == true);
    delete textTool;
}

void TestChangeTrackedDelete::testTableDelete()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    KoTextEditor *textEditor = textTool->textEditor();
    QVERIFY(textEditor);
    QTextDocument *document = textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    QTextCursor *cursor = textEditor->cursor();
    insertSampleTable(document);

    cursor->setPosition(13);
    cursor->setPosition(102, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(13).unicode(), (ushort)(QChar::ObjectReplacementCharacter));

    // This is wierd. Without this loop present the succeeding call to inlineTextObject returs NULL. Why ??????
    for (int i=0; i<document->characterCount(); i++) {
        cursor->setPosition(i);
    }

    cursor->setPosition(14);
    KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(*cursor));
    QTextDocumentFragment deleteData =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();

    QTextDocument deleteDocument;
    QTextCursor deleteCursor(&deleteDocument);

    deleteCursor.insertFragment(deleteData);
    bool tableFound = false;

    for (int i=0; i < deleteDocument.characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentTable()) {
            tableFound = true;
            break;
        }
    }
    QVERIFY(tableFound == true);
    QTextTable *table = deleteCursor.currentTable();
    QVERIFY(table->rows() == 3);
    QVERIFY(table->columns() == 3);

    tableFound = false;
    for (int i=0; i < document->characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentTable()) {
            tableFound = true;
            break;
        }
    }
    QVERIFY(tableFound == false);

    delCommand->undo();
    tableFound = false;
    for (int i=0; i < document->characterCount(); i++) {
        deleteCursor.setPosition(i);
        if (deleteCursor.currentTable()) {
            tableFound = true;
            break;
        }
    }
    QVERIFY(tableFound == true);
    delete textTool;
}

void TestChangeTrackedDelete::insertSampleList(QTextDocument *document)
{
    QTextCursor cursor(document);
    KoList *list = new KoList(document, new KoListStyle());
    cursor.insertText("This is a paragraph of text before a list.");
    cursor.insertBlock();
    list->add(cursor.block(), 1);
    cursor.insertText("This is a list-item 1");
    cursor.insertBlock();
    list->add(cursor.block(), 1);
    cursor.insertText("This is a list-item 2");
    cursor.insertBlock();
    list->add(cursor.block(), 1);
    cursor.insertText("This is a list-item 3");
    cursor.insertBlock();
    list->add(cursor.block(), 1);
    cursor.insertText("This is a list-item 4");
    cursor.insertBlock();
    list->add(cursor.block(), 1);
    cursor.insertText("This is a list-item 5");
}

void TestChangeTrackedDelete::insertSampleTable(QTextDocument *document)
{
    QTextCursor cursor(document);
    cursor.insertText("This is a paragraph of text before a table.");
    QTextTable *table = cursor.insertTable(3,3);
    table->cellAt(0,0).firstCursorPosition().insertText("first");
    table->cellAt(0,1).firstCursorPosition().insertText("second");
    table->cellAt(0,2).firstCursorPosition().insertText("third");
    table->cellAt(1,0).firstCursorPosition().insertText("fourth");
    table->cellAt(1,1).firstCursorPosition().insertText("fifth");
    table->cellAt(1,2).firstCursorPosition().insertText("sixth");
    table->cellAt(2,0).firstCursorPosition().insertText("seventh");
    table->cellAt(2,1).firstCursorPosition().insertText("eigth");
    table->cellAt(2,2).firstCursorPosition().insertText("ninth");
}

QTEST_MAIN(TestChangeTrackedDelete)

#include <TestChangeTrackedDelete.moc>
