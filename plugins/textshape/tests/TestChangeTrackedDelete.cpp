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

TestChangeTrackedDelete::TestChangeTrackedDelete()
{
}

TestChangeTrackedDelete::~TestChangeTrackedDelete()
{
}

void TestChangeTrackedDelete::testSimpleDelete()
{
    TextTool *tool = new TextTool(new MockCanvas);
    QTextDocument *document = tool->m_textEditor->document();
    QTextCursor *cursor = tool->m_textEditor->cursor();
    cursor->insertText("Hello World");
    cursor->setPosition(4);
    ChangeTrackedDeleteCommand *delCommand = new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::PreviousChar, tool);
    tool->m_textEditor->addCommand(delCommand);
    QCOMPARE(document->characterAt(3).unicode(), (ushort)(QChar::ObjectReplacementCharacter));
    delCommand->undo();
    QCOMPARE(document->characterAt(3), QChar('l'));
}

QTEST_MAIN(TestChangeTrackedDelete)

#include <TestChangeTrackedDelete.moc>
