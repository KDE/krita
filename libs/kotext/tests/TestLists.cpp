#include "TestDocumentLayout.h"

#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
//#include "KoTextBlockBorderData.h"

#include <QtGui>

//#include <kdebug.h>

void TestDocumentLayout::testBasicList() {
    initForNewTest("Base\nListItem\nListItem2: The quick brown fox jums over the lazy dog.\nNormal\nNormal");

    KoParagraphStyle style;
    QTextBlock block = doc->begin();
    style.applyStyle(block);
    block = block.next();
    QVERIFY(block.isValid());

    KoListStyle listStyle;
    style.setListStyle(listStyle);
    style.applyStyle(block); // make this a listStyle
    QVERIFY(block.textList());
    QVERIFY(block.textList()->format().intProperty(QTextListFormat::ListStyle) == KoListStyle::DiscItem);
    block = block.next();
    QVERIFY(block.isValid());
    style.applyStyle(block); // make this a listStyle

    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);
    block = doc->begin().next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 2
    QCOMPARE(blockLayout->lineAt(0).x(), 12.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 3
    QCOMPARE(blockLayout->lineAt(0).x(), 12.0);
    QVERIFY(blockLayout->lineCount() > 1);
    QCOMPARE(blockLayout->lineAt(1).x(), 12.0); // make sure not only the first line is indented
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 4
    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);
}

void TestDocumentLayout::testNumberedList() {
    initForNewTest("Base\nListItem1\nListItem2\nListItem3\nListItem4\nListItem5\nListItem6\nListItem6\nListItem7\nListItem8\nListItem9\nListItem10\nListItem11\nListItem12\n");

    KoParagraphStyle style;
    styleManager->add(&style);
    QTextBlock block = doc->begin();
    style.applyStyle(block);
    block = block.next();
    QVERIFY(block.isValid());

    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    style.setListStyle(listStyle);

    for(int i=1; i <= 9; i++) {
        QVERIFY(block.isValid());
        style.applyStyle(block);
        //qDebug() << "->" << block.text();
        block = block.next();
    }
    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);
    QTextBlock blok = doc->begin().next();
    double indent = blok.layout()->lineAt(0).x();
    QVERIFY(indent > 0.0);
    for(int i=1; i <= 9; i++) {
        //qDebug() << "=>" << blok.text();
        QCOMPARE(blok.layout()->lineAt(0).x(), indent); // all the same indent.
        blok = blok.next();
    }

    // now make number of listitems be more than 10, so we use 2 digits.
    for(int i=9; i <= 12; i++) {
        QVERIFY(block.isValid());
        style.applyStyle(block);
        //qDebug() << "->" << block.text();
        block = block.next();
    }
    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);
    blok = doc->begin().next();
    double indent2 = blok.layout()->lineAt(0).x();
    QVERIFY(indent2 > indent); // since it takes an extra digit
    for(int i=2; i <= 12; i++) {
        //qDebug() << "=>" << blok.text();
        QCOMPARE(blok.layout()->lineAt(0).x(), indent2); // all the same indent.
        blok = blok.next();
    }

    // now to make sure the text is actually properly set.
    block = doc->begin().next();
    int i=1;
    while(block.isValid() && i < 14) {
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        QVERIFY(data);
        QCOMPARE(data->counterText(), QString::number(i++));
        block = block.next();
    }

    listStyle.setListItemSuffix(".");
    listStyle.setStartValue(4);
    style.setListStyle(listStyle);

    QTextCursor cursor(doc);
    cursor.setPosition(40); // listItem4
    block = cursor.block();
    QVERIFY(block.textList() != 0);
    style.applyStyle(block);
    QCOMPARE(block.textList()->format().intProperty(KoListStyle::StartValue), 4);

    QTextBlockFormat format = cursor.blockFormat();
    format.setProperty(KoListStyle::ExplicitListValue, 12);
    cursor.setBlockFormat(format);

    // at this point we start numbering at 4. Have 4, 5, 6, 12, 13, 14, 15 etc
    layout->layout();

    // now to make sur the text is actually properly set.
    block = doc->begin().next();
    i=4;
    while(block.isValid() && i < 22) {
        if(i == 7) i = 12;
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        QVERIFY(data);
        QCOMPARE(data->counterText(), QString::number(i++) + ".");
        block = block.next();
    }
}

void TestDocumentLayout::testInterruptedLists() {
    initForNewTest("ListItem1\nListItem2\nNormal Parag\nAnother parag\nListItem3\n");

    KoParagraphStyle style;
    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    listStyle.setConsecutiveNumbering(true);
    listStyle.setListItemSuffix(".");
    style.setListStyle(listStyle);

    QTextBlock block = doc->begin();
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);
    block = block.next();
    block = block.next();
    block = block.next();
    style.applyStyle(block);

    layout->layout();

    block = doc->begin();
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QVERIFY(data->counterText() == "1.");
    block = block.next();
    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QVERIFY(data->counterText() == "2.");
    block = block.next();
    QCOMPARE(block.layout()->lineAt(0).x(), 0.0);
    QVERIFY(block.userData() ==  0);
    block = block.next();
    QCOMPARE(block.layout()->lineAt(0).x(), 0.0);
    QVERIFY(block.userData() ==  0);
    block = block.next(); // list item 3
    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QVERIFY(data->counterText() == "3.");

    // now the other way around
    block = doc->begin();
    listStyle.setConsecutiveNumbering(false);
    listStyle.applyStyle(block);
    layout->layout();

    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QVERIFY(data->counterText() == "1.");
    block = block.next();
    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QVERIFY(data->counterText() == "2.");
    block = block.next();
    QCOMPARE(block.layout()->lineAt(0).x(), 0.0);
    QVERIFY(block.userData() ==  0);
    block = block.next();
    QCOMPARE(block.layout()->lineAt(0).x(), 0.0);
    QVERIFY(block.userData() ==  0);
    block = block.next(); // list item 3
    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    //qDebug() << data->counterText();
    QVERIFY(data->counterText() == "1.");
}

void TestDocumentLayout::testNestedLists() {
    initForNewTest("Root\nplants\nherbs\ncinnamon\ncurry\nroses\nhumans\nFrank\nAnkje\nOther\nSkip\nLastItem");

    KoParagraphStyle h1;
    styleManager->add(&h1);
    KoParagraphStyle h2;
    styleManager->add(&h2);
    KoParagraphStyle h3;
    styleManager->add(&h3);
    KoParagraphStyle h4;
    styleManager->add(&h4);

    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    listStyle.setLevel(1);
    h1.setListStyle(listStyle);
    listStyle.setLevel(2);
    listStyle.setListItemSuffix(".");
    h2.setListStyle(listStyle);
    listStyle.setLevel(3);
    listStyle.setListItemSuffix("");
    h3.setListStyle(listStyle);
    listStyle.setLevel(4);
    listStyle.setDisplayLevel(2);
    h4.setListStyle(listStyle);

    QTextBlock block = doc->begin().next();
    h1.applyStyle(block);
    block = block.next();
    h2.applyStyle(block);
    block = block.next();
    h3.applyStyle(block);
    block = block.next();
    h3.applyStyle(block);
    block = block.next(); // roses
    h2.applyStyle(block);
    block = block.next();
    h1.applyStyle(block); // humans
    block = block.next();
    h2.applyStyle(block);
    block = block.next();
    h2.applyStyle(block);
    block = block.next();
    h1.applyStyle(block);
    block = block.next();
    h3.applyStyle(block); // notice missing h2
    block = block.next();
    QVERIFY(block.isValid());
    h4.applyStyle(block);

    layout->layout();

    block = doc->begin();
    QVERIFY(block.userData() == 0);
    block = block.next();
    static const char* texts[] = { "1", "1.1.", "1.1.1", "1.1.2", "1.2.", "2", "2.1.", "2.2.", "3", "3.0.1", "1.1" };
    int i=0;
    double indent=0.0;
    while(block.isValid()) {
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        //qDebug() << "text: " << block.text();
        //qDebug() << "expected: " << texts[i];
        QVERIFY(data);
        if(i < 3) {
            //qDebug() << "indent:" << data->counterWidth();
            QVERIFY (indent < data->counterWidth()); // deeper indent, larger width
            indent = data->counterWidth();
        }
        //qDebug() << data->counterText();
        QVERIFY(data->counterText() == texts[i++]);
        block = block.next();
    }
}

void TestDocumentLayout::testAutoRestartList() {
    initForNewTest("Humans\nGhandi\nEinstein\nInventions\nCar\nToilet\nLaboratory\n");

    KoParagraphStyle h1;
    styleManager->add(&h1);
    KoParagraphStyle h2;
    styleManager->add(&h2);

    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    listStyle.setLevel(1);
    h1.setListStyle(listStyle);

    KoListStyle listStyle2;
    listStyle2.setStyle(KoListStyle::DecimalItem);
    listStyle2.setLevel(2);
    h2.setListStyle(listStyle2);

    QTextBlock block = doc->begin();
    h1.applyStyle(block);
    block = block.next(); h2.applyStyle(block);
    block = block.next(); h2.applyStyle(block);
    block = block.next(); h1.applyStyle(block); // inventions
    block = block.next(); h2.applyStyle(block);
    QTextBlock car = block;
    block = block.next(); h2.applyStyle(block);
    block = block.next(); h2.applyStyle(block);

    layout->layout();

    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    qDebug() << data->counterText();
    QCOMPARE(data->counterText(), QString("2.1"));
}

void TestDocumentLayout::testListParagraphIndent() {
    // test that the list item is drawn indented on an indented paragraph.
    initForNewTest("Foo\nBar\n");

    KoParagraphStyle h1;
    styleManager->add(&h1);
    KoParagraphStyle h2;
    styleManager->add(&h2);
    h2.setTextIndent(10);

    KoListStyle listStyle;
    h1.setListStyle(listStyle);
    KoListStyle listStyle2;
    h2.setListStyle(listStyle2);

    QTextBlock block = doc->begin();
    h1.applyStyle(block);
    block = block.next();
    h2.applyStyle(block);

    layout->layout();

    // still at h2 parag!
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    QVERIFY(data);
    QCOMPARE(data->counterPosition(), QPointF(10, 14.4));
}

void TestDocumentLayout::testRomanNumbering() {
    // Create, say 25 parags. layout. Then look to see if the items are proper roman numerals.
}

void TestDocumentLayout::testUpperAlphaNumbering() {
    // Create, say 27 parags. layout. Then look to see if the items are proper A B C D
}

void TestDocumentLayout::testRestartNumbering() {
    // create 5 items; restart the 3th. Check numbering.
    initForNewTest("a\nb\na\nb\nc\n");

    KoParagraphStyle h1;
    styleManager->add(&h1);
    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    h1.setListStyle(listStyle);

    QTextBlock block = doc->begin();
    while(block.isValid()) {
        h1.applyStyle(block);
        block = block.next();
    }

    QTextCursor cursor(doc);
    cursor.setPosition(5);
    QCOMPARE(cursor.block().text(), QString("a"));
    QTextBlockFormat format = cursor.blockFormat();
    format.setProperty(KoParagraphStyle::RestartListNumbering, true);
    cursor.setBlockFormat(format);

    layout->layout();

    static const char *values[] = { "1", "2", "1", "2", "3" };
    block = doc->begin();
    int i=0;
    while(block.isValid()) {
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        QVERIFY(data);
        QCOMPARE(data->counterText(), QString(values[i++]));

        block = block.next();
    }
}
