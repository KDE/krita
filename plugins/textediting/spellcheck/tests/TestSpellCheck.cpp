#include "TestSpellCheck.h"

#include "../BgSpellCheck.h"

#include <KoCharacterStyle.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextCharFormat>

class MySpellCheck : public BgSpellCheck
{
public:
    MySpellCheck(QObject *parent = 0) : BgSpellCheck(parent)
    {
    }

    QString publicFetchMoreText() {
        return fetchMoreText();
    }
    virtual void start() { }
};

void TestSpellCheck::testFetchMoreText()
{
    MySpellCheck checker;
    QTextDocument doc;
    QString text("some simple text\na second parag with more text\n");
    doc.setPlainText(text);

    checker.startRun(&doc, 0, text.size());
    QTextBlock block = doc.begin();
    QCOMPARE(checker.publicFetchMoreText(), block.text());
    block = block.next();
    QVERIFY(block.isValid());
    QCOMPARE(checker.publicFetchMoreText(), block.text());

    QTextCursor cursor(&doc);
    QTextCharFormat cf;
    cf.setProperty(KoCharacterStyle::Language, QVariant("pl"));
    cursor.setPosition(4, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(cf);

    checker.startRun(&doc, 0, text.size());
    block = doc.begin();
    QCOMPARE(checker.publicFetchMoreText(), QString("some"));
    QCOMPARE(checker.publicFetchMoreText(), block.text().mid(4));
    block = block.next();
    QVERIFY(block.isValid());
    QCOMPARE(checker.publicFetchMoreText(), block.text());

    // add some more
    cursor.setPosition(block.position() + 2);
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor); // 'second'
    int position2 = cursor.anchor();
    int position3 = cursor.position();
    cf.setProperty(KoCharacterStyle::Language, QVariant("br"));
    cursor.mergeCharFormat(cf);
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, 2);
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor); // 'with'
    cf.setProperty(KoCharacterStyle::Language, QVariant("en"));
    cursor.mergeCharFormat(cf);

    checker.startRun(&doc, 0, text.size());
    checker.setDefaultLanguage("en_NZ");
    block = doc.begin();
    QCOMPARE(checker.publicFetchMoreText(), QString("some"));
    QCOMPARE(checker.publicFetchMoreText(), block.text().mid(4));
    block = block.next();
    QVERIFY(block.isValid());
    QCOMPARE(checker.publicFetchMoreText(), block.text().left(2));
    QCOMPARE(checker.publicFetchMoreText(), text.mid(position2, position3 - position2));
    QCOMPARE(checker.publicFetchMoreText(), text.mid(position3).trimmed());

    // start a check in the middle of a block
    checker.startRun(&doc, 4, 19);
    block = doc.begin();
    QCOMPARE(checker.publicFetchMoreText(), block.text().mid(4));
    block = block.next();
    QVERIFY(block.isValid());
    QCOMPARE(checker.publicFetchMoreText(), block.text().left(2));
}

void TestSpellCheck::testFetchMoreText2()
{
    MySpellCheck checker;
    QTextDocument doc;
    doc.setPlainText("\n\n\nMostly Empty Parags.\n\n");
    checker.startRun(&doc, 0, 20);
    checker.setDefaultLanguage("en_NZ");
    QCOMPARE(checker.publicFetchMoreText(), QString("Mostly Empty Parags."));
}

QTEST_KDEMAIN(TestSpellCheck, GUI)

#include <TestSpellCheck.moc>
