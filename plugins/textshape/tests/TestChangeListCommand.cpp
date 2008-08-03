#include "TestChangeListCommand.h"
#include "../commands/ChangeListCommand.h"

#include <KoListStyle.h>
#include <KoListLevelProperties.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QTextList>

void TestChangeListCommand::addList() {
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");

    QTextBlock block = doc.begin().next();
    ChangeListCommand clc(block, KoListStyle::DecimalItem);
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

    ChangeListCommand clc2(block, KoListStyle::DiscItem);
    clc2.redo();

    block = doc.begin();
    QVERIFY(block.textList() == 0);
    block = block.next();
    QCOMPARE(block.textList(), tl);
    block = block.next();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
}

void TestChangeListCommand::removeList() {
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KoListStyle style;
    QTextBlock block = doc.begin().next();
    while(block.isValid()) {
        style.applyStyle(block);
        block = block.next();
    }

    block = doc.begin().next();
    QVERIFY(block.textList()); // init, we should not have to test KoListStyle here ;)

    ChangeListCommand clc(block, KoListStyle::NoItem);
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

    ChangeListCommand clc2(block, KoListStyle::NoItem);
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

void TestChangeListCommand::joinList() {
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4\n");
    KoListStyle style;
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

    ChangeListCommand clc(block, KoListStyle::DecimalItem);
    clc.redo();
    QCOMPARE(block.textList(), tl);
}

void TestChangeListCommand::joinList2() {
    // test usecase of joining with the one before and the one after based on similar styles.
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertText("Root\nparag1\nparag2\nparag3\nparag4");
    KoListStyle style;
    KoListLevelProperties llp1;
    llp1.setStyle(KoListStyle::DiscItem);
    style.setLevelProperties(llp1);
    QTextBlock block = doc.begin().next().next();
    style.applyStyle(block); // apply on parag2

    KoListStyle style2;
    KoListLevelProperties llp;
    llp.setStyle(KoListStyle::DecimalItem);
    style2.setLevelProperties(llp);
    block = block.next().next(); // parag4
    style2.applyStyle(block);

    // now apply the default 'DiscItem' on 'parag1' expecting it to join with the list already set on 'parag2'
    block = doc.begin().next();
    ChangeListCommand clc(block, KoListStyle::DiscItem);
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
    ChangeListCommand clc2(block, KoListStyle::DecimalItem);
    clc2.redo();
    QVERIFY(block.textList());
    QVERIFY(block.textList() != tl);
    QVERIFY(block.textList() == numberedList);
    QCOMPARE(numberedList->format().intProperty(QTextListFormat::ListStyle), (int) KoListStyle::DecimalItem);
}

void TestChangeListCommand::splitList() {
    // assume I start with;
    // 1 paragA
    // 1.1 paragB
    // 1.2 paragC
    // now I change parag 'B' to '1.a'  then C should have 1.1 as a numbering. I.e. we should split an existing list.

    QTextDocument doc;
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
    QVERIFY (paragA.textList());
    QTextBlock paragB = paragA.next();
    QVERIFY (paragB.textList());
    QVERIFY (paragB.textList() != paragA.textList());
    QTextBlock paragC = paragB.next();
    QVERIFY (paragC.textList());
    QCOMPARE (paragC.textList(), paragB.textList());

    QTextList *tl = paragB.textList();
    ChangeListCommand clc(paragB, KoListStyle::AlphaLowerItem);
    clc.redo();

    QVERIFY(doc.begin().textList() == 0);
    QVERIFY(paragA.textList());
    QTextList *newTextList = paragB.textList();
    QVERIFY(newTextList != tl);
    QCOMPARE(paragC.textList(), tl);

    QCOMPARE(tl->format().intProperty(KoListStyle::Level), 2);
    QCOMPARE(newTextList->format().intProperty(KoListStyle::Level), 2);
}

QTEST_MAIN(TestChangeListCommand)

#include <TestChangeListCommand.moc>
