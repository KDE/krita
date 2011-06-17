/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2009-2010 Casper Boemann <casper.boemann@kogmbh.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestDocumentLayout.h"
#include "MockRootAreaProvider.h"
#include <QtGui>

#include <kdebug.h>
#include <kcomponentdata.h>

#include <KoTextDocument.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoTextLayoutRootArea.h>
#include <KoShape.h>

void TestDocumentLayout::initTestCase()
{
    m_doc = 0;
    m_layout = 0;
}

#if 0
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
    doc->setHtml("<p>lsdjflkdsjf lsdkjf lsdlflksejrl sdflsd flksjdf lksjrpdslfjfsdhtwkr[ivxxmvlwerponldsjf;dslflkjsorindfsn;epsdf</p><p>sldkfnwerpodsnf</p><p>sldkjfnpqwrdsf</p>");
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
#endif

void TestDocumentLayout::setupTest(const QString &initText)
{
    m_doc = new QTextDocument;
    Q_ASSERT(m_doc);

    MockRootAreaProvider *provider = new MockRootAreaProvider();
    Q_ASSERT(provider);
    KoTextDocument(m_doc).setInlineTextObjectManager(new KoInlineTextObjectManager);

    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false)); //do it manually since we do not load the appDefaultStyle

    m_styleManager = new KoStyleManager();
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    m_layout = new KoTextDocumentLayout(m_doc, provider);
    Q_ASSERT(m_layout);
    m_doc->setDocumentLayout(m_layout);

    QTextBlock block = m_doc->begin();
    if (initText.length() > 0) {
        QTextCursor cursor(m_doc);
        cursor.insertText(initText);
        KoParagraphStyle style;
        style.setStyleId(101); // needed to do manually since we don't use the stylemanager
        QTextBlock b2 = m_doc->begin();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
}

void TestDocumentLayout::testRootAreaZeroWidth()
{
    setupTest("a");

    MockRootAreaProvider *provider = dynamic_cast<MockRootAreaProvider*>(m_layout->provider());
    provider->setSuggestedSize(QSizeF(0.,200.));

    m_layout->layout();

    QVERIFY(provider->m_askedForMoreThenOneArea);
    QVERIFY(provider->m_area);
    QVERIFY(!provider->m_area->isDirty());
    QVERIFY(provider->m_area->virginPage());
    QVERIFY(provider->m_area->nextStartOfArea());
    QVERIFY(provider->m_area->isStartingAt(provider->m_area->nextStartOfArea()));
    QCOMPARE(provider->m_area->boundingRect(), QRectF(0.,0.,0.,0.));
    QCOMPARE(provider->m_area->referenceRect(), QRectF(0.,0.,0.,0.));
}

void TestDocumentLayout::testRootAreaZeroHeight()
{
    setupTest("a");

    MockRootAreaProvider *provider = dynamic_cast<MockRootAreaProvider*>(m_layout->provider());
    provider->setSuggestedSize(QSizeF(200.,0.));

    m_layout->layout();

    QVERIFY(!provider->m_askedForMoreThenOneArea); // we add the text anyways even if it does not match in height
    QVERIFY(provider->m_area);
    QVERIFY(!provider->m_area->isDirty());
    QVERIFY(!provider->m_area->virginPage()); // should not be virigin any longer cause we added text
    QVERIFY(provider->m_area->nextStartOfArea());
    QVERIFY(!provider->m_area->isStartingAt(provider->m_area->nextStartOfArea())); // start- and end-iterator should not be equal cause we added text
    QCOMPARE(provider->m_area->boundingRect(), QRectF(0.,0.,200.,0.));
    QCOMPARE(provider->m_area->referenceRect(), QRectF(0.,0.,200.,0.));
}

void TestDocumentLayout::testRootAreaZeroWidthAndHeight()
{
    setupTest("a");

    MockRootAreaProvider *provider = dynamic_cast<MockRootAreaProvider*>(m_layout->provider());
    provider->setSuggestedSize(QSizeF(0.,0.));

    m_layout->layout();

    QVERIFY(provider->m_askedForMoreThenOneArea);
    QVERIFY(provider->m_area);
    QVERIFY(!provider->m_area->isDirty());
    QVERIFY(provider->m_area->virginPage());
    QVERIFY(provider->m_area->nextStartOfArea());
    QVERIFY(provider->m_area->isStartingAt(provider->m_area->nextStartOfArea()));
    QCOMPARE(provider->m_area->boundingRect(), QRectF(0.,0.,0.,0.));
    QCOMPARE(provider->m_area->referenceRect(), QRectF(0.,0.,0.,0.));
}

QTEST_KDEMAIN(TestDocumentLayout, GUI)

#include <TestDocumentLayout.moc>
