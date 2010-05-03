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
    QTextDocument *document = textTool->m_textEditor->document();
    QTextCursor *cursor = textTool->m_textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::PreviousChar, textTool);
    textTool->m_textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(3).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    delCommand->undo();
    QCOMPARE(document->characterAt(3), QChar('l'));
    delete textTool;
}

void TestChangeTrackedDelete::testDeleteNextChar()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    QTextDocument *document = textTool->m_textEditor->document();
    QTextCursor *cursor = textTool->m_textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textTool->m_textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(4).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    delCommand->undo();
    QCOMPARE(document->characterAt(4), QChar('o'));
    delete textTool;
}

void TestChangeTrackedDelete::testDeleteSelection()
{
    TextTool *textTool = new TextTool(new MockCanvas);
    QTextDocument *document = textTool->m_textEditor->document();
    KoTextDocument(document).changeTracker()->setDisplayChanges(true);
    QTextCursor *cursor = textTool->m_textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(2);
    cursor->setPosition(8, QTextCursor::KeepAnchor);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, textTool);
    textTool->m_textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(2).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    cursor->setPosition(3);
    int changeId = cursor->charFormat().intProperty(KoCharacterStyle::ChangeTrackerId);
    QTextDocumentFragment deleteData = KoTextDocument(document).changeTracker()->elementById(changeId)->getDeleteData();
    QCOMPARE(deleteData.toPlainText(), QString("llo Wo"));
    delete textTool;
}

QTEST_MAIN(TestChangeTrackedDelete)

#include <TestChangeTrackedDelete.moc>
