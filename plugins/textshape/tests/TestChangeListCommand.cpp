#include "TestChangeListCommand.h"
#include "tests/MockShapes.h"
#include "../commands/ChangeListCommand.h"

#include <KoListStyle.h>
#include <KoListLevelProperties.h>
#include <KoStyleManager.h>
#include <KoTextDocument.h>

#include <TextTool.h>
#include <KoCanvasBase.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QTextList>


void TestChangeListCommand::addList()
{
    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");

    QTextBlock block = doc.begin().next();
    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KoListStyle::DecimalItem);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList()); // this one we just changed
    QTextList *tl = block.textList();
    block = block.next();
    QVERIFY(block.textList() == 0);

    QTextListFormat format = tl->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int) KoListStyle::DecimalItem);

    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KoListStyle::DiscItem);
    clc2.setTool(tool);
    clc2.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QCOMPARE(block.textList(), tl);
    block = block.next();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
}

void TestChangeListCommand::addListWithLevel2()
{
    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");

    QTextBlock block = doc.begin().next();
    cursor.setPosition(block.position());

    KoListStyle style;
    KoListLevelProperties llp;
    llp.setLevel(2);
    llp.setDisplayLevel(2);
    llp.setStyle(KoListStyle::DiscItem);
    style.setLevelProperties(llp);

    ChangeListCommand clc(cursor, &style, 2);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList()); // this one we just changed
    QTextList *tl = block.textList();
    block = block.next();
    QVERIFY(block.textList() == 0);

    QTextListFormat format = tl->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int) KoListStyle::DiscItem);
    QCOMPARE(format.intProperty(KoListStyle::DisplayLevel), (int) 2);
    QCOMPARE(format.intProperty(KoListStyle::Level), (int) 2);
}

void TestChangeListCommand::removeList()
{
    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KoListStyle style;
    QTextBlock block = doc.begin().next();
    while (block.isValid()) {
        style.applyStyle(block);
        block = block.next();
    }

    block = doc.begin().next();
    QVERIFY(block.textList()); // init, we should not have to test KoListStyle here ;)

    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KoListStyle::None);
    clc.setTool(tool);
    clc.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());

    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KoListStyle::None);
    clc2.setTool(tool);
    clc2.redo();
    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList());
    block = block.next();
    QVERIFY(block.textList() == 0);
}

void TestChangeListCommand::joinList()
{
    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KoListStyle style;
    KoListLevelProperties llp;
    llp.setLevel(1);
    llp.setStyle(KoListStyle::DiscItem);
    style.setLevelProperties(llp);
    QTextBlock block = doc.begin().next();
    style.applyStyle(block);
    block = block.next();
    block = block.next(); // skip parag2
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);

    block = doc.begin().next();
    QTextList *tl = block.textList();
    QVERIFY(tl); // init, we should not have to test KoListStyle here ;)
    block = block.next(); // parag2
    QVERIFY(block.textList() == 0);

    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KoListStyle::DiscItem);
    clc.setTool(tool);
    clc.redo();
    QCOMPARE(block.textList(), tl);
}

void TestChangeListCommand::joinList2()
{
    // test usecase of joining with the one before and the one after based on similar styles.
    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4");
    KoListStyle style;
    KoListLevelProperties llp1;
    llp1.setLevel(1);
    llp1.setStyle(KoListStyle::DiscItem);
    style.setLevelProperties(llp1);
    QTextBlock block = doc.begin().next().next();
    style.applyStyle(block); // apply on parag2

    KoListStyle style2;
    KoListLevelProperties llp;
    llp.setLevel(1);
    llp.setStyle(KoListStyle::DecimalItem);
    llp.setListItemSuffix(".");
    style2.setLevelProperties(llp);
    block = block.next().next(); // parag4
    style2.applyStyle(block);

    // now apply the default 'DiscItem' on 'parag1' expecting it to join with the list already set on 'parag2'
    block = doc.begin().next();
    cursor.setPosition(block.position());
    ChangeListCommand clc(cursor, KoListStyle::DiscItem);
    clc.setTool(tool);
    clc.redo();
    QTextList *tl = block.textList();
    QVERIFY(tl);
    block = block.next();
    QCOMPARE(tl, block.textList());
    QCOMPARE(tl->format().intProperty(QTextListFormat::ListStyle), (int) KoListStyle::DiscItem);

    // now apply the 'DecimalItem' on 'parag3' and expect it to join with the list already set on 'parag4'
    block = doc.end().previous(); // parag4
    QCOMPARE(block.text(), QString("parag4"));
    QTextList *numberedList = block.textList();
    QVERIFY(numberedList);
    block = block.previous(); // parag3
    QVERIFY(block.textList() == 0);
    cursor.setPosition(block.position());
    ChangeListCommand clc2(cursor, KoListStyle::DecimalItem);
    clc2.setTool(tool);
    clc2.redo();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
    QVERIFY(block.textList() == numberedList);
    QCOMPARE(numberedList->format().intProperty(QTextListFormat::ListStyle), (int) KoListStyle::DecimalItem);
}

void TestChangeListCommand::splitList()
{
    // assume I start with;
    // 1 paragA
    // 1.1 paragB
    // 1.2 paragC
    // now I change parag 'B' to '1.a'  then C should have 1.1 as a numbering. I.e. we should split an existing list.

    QTextDocument doc;
    KoTextDocument(&doc).setStyleManager(new KoStyleManager);
    TextTool *tool = new TextTool(new MockCanvas);
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparagA\nparagB\nparagC");
    QTextBlock block = doc.begin().next();
    KoListStyle style;
    style.applyStyle(block); // apply on parag2

    KoListStyle style2;
    KoListLevelProperties llp = style2.levelProperties(2);
    style2.setLevelProperties(llp);
    block = block.next();
    style2.applyStyle(block);
    block = block.next();
    style2.applyStyle(block);

    QTextBlock paragA = doc.begin().next();
    QVERIFY(paragA.textList());
    QTextBlock paragB = paragA.next();
    QVERIFY(paragB.textList());
    QVERIFY(paragB.textList() != paragA.textList());
    QTextBlock paragC = paragB.next();
    QVERIFY(paragC.textList());
    QCOMPARE(paragC.textList(), paragB.textList());

    QTextList *tl = paragB.textList();
    cursor.setPosition(paragB.position());
    ChangeListCommand clc(cursor, KoListStyle::AlphaLowerItem);
    clc.setTool(tool);
    clc.redo();

    QVERIFY(doc.begin().textList() == 0);
    QVERIFY(paragA.textList());
    QTextList *newTextList = paragB.textList();
    QVERIFY(newTextList == tl);
    QCOMPARE(paragC.textList(), tl);

    QCOMPARE(tl->format().intProperty(KoListStyle::Level), 2);
    QCOMPARE(newTextList->format().intProperty(KoListStyle::Level), 2);
}

QTEST_MAIN(TestChangeListCommand)

#include <TestChangeListCommand.moc>
