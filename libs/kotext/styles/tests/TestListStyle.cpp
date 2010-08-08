#include "TestListStyle.h"

#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextBlockBorderData.h"
#include "KoTextDocument.h"
#include "styles/KoStyleManager.h"

#include <QTextDocument>
#include <QTextCursor>

void TestListStyle::testListStyle()
{
    KoListStyle ls;
    KoListLevelProperties llp = ls.levelProperties(2);
    QCOMPARE(llp.level(), 2);

    llp.setStyle(KoListStyle::AlphaLowerItem);
    KoListLevelProperties llp2 = ls.levelProperties(2);
    QVERIFY(llp2.style() != llp.style());

    ls.setLevelProperties(llp);
    QCOMPARE(llp.level(), 2);
    QCOMPARE(llp.style(), KoListStyle::AlphaLowerItem);

    llp = ls.levelProperties(2);
    QCOMPARE(llp.level(), 2);
    QCOMPARE(llp.style(), KoListStyle::AlphaLowerItem);

    QTextDocument doc;
    KoTextDocument kodoc(&doc);
    kodoc.setStyleManager(new KoStyleManager);
    QTextCursor cursor(&doc);
    cursor.insertText("foo\nbar\nBaz\n");
    QTextBlock block = doc.begin();
    ls.applyStyle(block, 2);
    QVERIFY(block.textList());
    QTextList *textList = block.textList();
    QTextListFormat format = textList->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int)(KoListStyle::AlphaLowerItem));

    block = block.next();
    QVERIFY(block.isValid());
    ls.applyStyle(block, 2);
    QVERIFY(block.textList());
    QCOMPARE(block.textList(), textList);

    ls.applyStyle(block, 10); // should set the properties of the only one that is set, level 1
    QVERIFY(block.textList());
    textList = block.textList();
    format = textList->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int)(KoListStyle::AlphaLowerItem));

}

QTEST_MAIN(TestListStyle)
#include <TestListStyle.moc>
