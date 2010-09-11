#include "TestDocumentLayout.h"
#include <QtGui>

#include <kdebug.h>
#include <kcomponentdata.h>

void TestDocumentLayout::initTestCase()
{
    shape1 = 0;
    doc = 0;
    layout = 0;
}

void TestDocumentLayout::initForNewTest()
{
    // this leaks memory like mad, but who cares ;)
    shape1 = new MockTextShape();
    shape1->setSize(QSizeF(200, 1000));

    // this leaks memory like mad, but who cares ;)
    doc = shape1->layout->document();
    Q_ASSERT(doc);
    layout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
    Q_ASSERT(layout);
}

void TestDocumentLayout::testHitTest()
{
    // init a basic document with 3 parags.
    initForNewTest();
    doc->setHtml("<p>lsdjflkdsjf lsdkjf lsdlflksejrl sdflsd flksjdf lksjrpdslfjfsdhtwkr[ivxxmvlwerponldsjf;dslflkjsorindfsn;epsdf</p><p>sldkfnwerpodsnf</p><p>sldkjfnpqwrdsf</p>");
    QTextBlock block = doc->begin();
    qreal offset = 50.0;
    qreal lineHeight = 0;
    int lines = 0, parag = 0;
    qreal paragOffets[3];
    while (1) {
        if (!block.isValid()) break;
        paragOffets[parag++] = offset;
        QTextLayout *txtLayout = block.layout();
        txtLayout->beginLayout();
        while (1) {
            QTextLine line = txtLayout->createLine();
            if (!line.isValid()) break;
            lines++;
            line.setLineWidth(50);
            line.setPosition(QPointF(20, offset));
            offset += 20;
            lineHeight = line.height();
        }
        txtLayout->endLayout();
        block = block.next();
    }
    //QCOMPARE(lines, 7);

    // outside text
    QCOMPARE(layout->hitTest(QPointF(0, 0), Qt::FuzzyHit), 0);
    QCOMPARE(layout->hitTest(QPointF(0, 0), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(19, 49), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(71, 0), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(71, 51), Qt::ExactHit), -1);

    // first char
    QCOMPARE(layout->hitTest(QPointF(20, 51), Qt::ExactHit), 0);
    QCOMPARE(layout->hitTest(QPointF(20, 50), Qt::ExactHit), 0);

    // below line 1
    QCOMPARE(layout->hitTest(QPointF(20, 51 + lineHeight), Qt::ExactHit), -1);
    QVERIFY(layout->hitTest(QPointF(20, 51 + lineHeight), Qt::FuzzyHit) > 0); // line 2

    // parag2
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::ExactHit), 109);
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::FuzzyHit), 109);
    QVERIFY(layout->hitTest(QPointF(20, paragOffets[1] + 20), Qt::FuzzyHit) > 109);
}

void TestDocumentLayout::testHitTestSection()
{
    // init a basic document with 3 parags.
    initForNewTest();
    QTextCursor cursor;
    QTextBlock block = doc->begin();
    qreal offset = 50.0;
    qreal lineHeight = 0;
    int lines = 0, parag = 0;
    qreal paragOffets[3];
    while (1) {
        if (!block.isValid()) break;
        paragOffets[parag++] = offset;
        QTextLayout *txtLayout = block.layout();
        txtLayout->beginLayout();
        while (1) {
            QTextLine line = txtLayout->createLine();
            if (!line.isValid()) break;
            lines++;
            line.setLineWidth(50);
            line.setPosition(QPointF(20, offset));
            offset += 20;
            lineHeight = line.height();
        }
        txtLayout->endLayout();
        block = block.next();
    }
    //QCOMPARE(lines, 7);

    // outside text
    QCOMPARE(layout->hitTest(QPointF(0, 0), Qt::FuzzyHit), 0);
    QCOMPARE(layout->hitTest(QPointF(0, 0), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(19, 49), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(71, 0), Qt::ExactHit), -1);
    QCOMPARE(layout->hitTest(QPointF(71, 51), Qt::ExactHit), -1);

    // first char
    QCOMPARE(layout->hitTest(QPointF(20, 51), Qt::ExactHit), 0);
    QCOMPARE(layout->hitTest(QPointF(20, 50), Qt::ExactHit), 0);

    // below line 1
    QCOMPARE(layout->hitTest(QPointF(20, 51 + lineHeight), Qt::ExactHit), -1);
    QVERIFY(layout->hitTest(QPointF(20, 51 + lineHeight), Qt::FuzzyHit) > 0); // line 2

    // parag2
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::ExactHit), 109);
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::FuzzyHit), 109);
    QVERIFY(layout->hitTest(QPointF(20, paragOffets[1] + 20), Qt::FuzzyHit) > 109);
}

QTEST_KDEMAIN(TestDocumentLayout, GUI)

#include <TestDocumentLayout.moc>
