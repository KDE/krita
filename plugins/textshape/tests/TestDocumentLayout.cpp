/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#include <KoParagraphStyle.h>
#include <KoListStyle.h>
#include <KoListLevelProperties.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoTextDocument.h>

#include <QtGui>

#include <kdebug.h>
#include <kcomponentdata.h>

#define FRAME_SPACING 10.0


void TestDocumentLayout::initTestCase()
{
    m_shape1 = 0;
    m_doc = 0;
    m_layout = 0;
    m_blockLayout = 0;

    m_loremIpsum = QString("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");
}

void TestDocumentLayout::initForNewTest(const QString &initText)
{
    // this leaks memory like mad, but who cares ;)
    m_shape1 = new MockTextShape();
    m_shape1->setSize(QSizeF(200, 1000));

    // this leaks memory like mad, but who cares ;)
    m_layout = m_shape1->layout;
    Q_ASSERT(m_layout);
    m_doc = m_layout->document();
    Q_ASSERT(m_doc);
    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false)); //do it manually since we do not load the appDefaultStyle
    m_textLayout = new Layout(m_layout);
    m_layout->setLayout(m_textLayout);
    m_styleManager = new KoStyleManager();
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

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
    m_blockLayout = block.layout();
}

void TestDocumentLayout::testLineBreaking()
{
    initForNewTest(m_loremIpsum);
    m_layout->layout();

    //QCOMPARE(m_blockLayout->lineCount(), 16);
    QCOMPARE(m_blockLayout->lineForTextPosition(1).width(), 200.0);
}

void TestDocumentLayout::testMultiFrameLineBreaking()
{
    initForNewTest(m_loremIpsum);
    m_shape1->setSize(QSizeF(200, 47)); // fits 3 lines.
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(120, 1000));
    m_layout->addShape(shape2);

    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).width(), 200.0);
    QCOMPARE(m_blockLayout->lineAt(1).width(), 200.0);
    QCOMPARE(m_blockLayout->lineAt(2).width(), 200.0);
    QCOMPARE(m_blockLayout->lineAt(3).width(), 120.0);
    QCOMPARE(m_blockLayout->lineAt(4).width(), 120.0);

    QTextLine line = m_blockLayout->lineAt(3);
    QVERIFY(line.y() > m_shape1->size().height()); // it has to be outside of m_shape1
    const qreal topOfFrame2 = line.y();
    line = m_blockLayout->lineAt(4);
    //qDebug() << line.y() - topOfFrame2 - 14.4;
    QVERIFY(qAbs(line.y() - topOfFrame2 - 14.4) < 0.125);
}

void TestDocumentLayout::testBasicLineSpacing()
{
    /// Tests incrementing Y pos based on the font size
    initForNewTest(m_loremIpsum);
    QTextCursor cursor(m_doc);
    cursor.setPosition(0);
    cursor.setPosition(m_loremIpsum.length() - 1, QTextCursor::KeepAnchor);
    QTextCharFormat charFormat = cursor.charFormat();
    charFormat.setFontPointSize(12);
    cursor.mergeCharFormat(charFormat);
    m_layout->layout();

    const qreal fontHeight12 = 12;
    qreal lineSpacing12 = fontHeight12 * 1.2; // 120% is the normal lineSpacing.
    const qreal fontHeight18 = 18;
    qreal lineSpacing18 = fontHeight18 * 1.2; // 120% is the normal lineSpacing.

    // QCOMPARE(m_blockLayout->lineCount(), 15);
    QCOMPARE(m_blockLayout->lineForTextPosition(1).width(), 200.0);
    QTextLine line;
    for (int i = 0; i < 15; i++) {
        line = m_blockLayout->lineAt(i);
        QVERIFY(line.isValid());
        // The reason for this weird check is that the values are stored internally
        // as 26.6 fixed point integers. The entire internal text layout is
        // actually done using fixed point arithmetic. This is due to embedded
        // considerations, and offers general performance benefits across all
        // platforms.
        //qDebug() << i << qAbs(line.y() - i * lineSpacing12);
        QVERIFY(qAbs(line.y() - i * lineSpacing12) < ROUNDING);
    }

    // make first word smaller, should have zero effect on lineSpacing.
    cursor.setPosition(0);
    cursor.setPosition(11, QTextCursor::KeepAnchor);
    charFormat.setFontPointSize(10);
    cursor.mergeCharFormat(charFormat);
    m_layout->layout();
    for (int i = 0; i < 15; i++) {
        line = m_blockLayout->lineAt(i);
        QVERIFY(line.isValid());
        //qDebug() << i << qAbs(line.y() - i * lineSpacing12);
        QVERIFY(qAbs(line.y() - i * lineSpacing12) < ROUNDING);
    }

    // make first word on second line word bigger, should move that line down a little.
    int pos = m_blockLayout->lineAt(1).textStart();
    cursor.setPosition(pos);
    cursor.setPosition(pos + 12, QTextCursor::KeepAnchor);
    charFormat.setFontPointSize(18);
    cursor.mergeCharFormat(charFormat);
    m_layout->layout();
    line = m_blockLayout->lineAt(0);
    QCOMPARE(line.y(), 0.0);
    line = m_blockLayout->lineAt(1);
    QVERIFY(qAbs(line.y() - lineSpacing12) < ROUNDING);

    for (int i = 2; i < 15; i++) {
        line = m_blockLayout->lineAt(i);
//qDebug() << "i: " << i << " gives: " << line.y() << " + " <<  line.ascent() << ", " << line.descent() << " = " << line.height();
        QVERIFY(qAbs(line.y() - (lineSpacing12 + lineSpacing18 + (i - 2) * lineSpacing12)) < ROUNDING);
    }
// Test widget to show what we have
    /*
        class Widget : public QWidget {
          public:
            Widget(KWTextDocumentLayout *layout) {
                m_layout = layout;
            }
            void paintEvent (QPaintEvent * e) {
                QPainter painter( this );
                QAbstractTextDocumentLayout::PaintContext pc;
                pc.cursorPosition = -1;
                m_layout->draw( &painter, pc);
            }
          private:
            KWTextDocumentLayout *m_layout;
        };

        QMainWindow mw;
        mw.setCentralWidget(new Widget(m_layout));
        mw.show();
        m_app->exec(); */
}

void TestDocumentLayout::testBasicLineSpacing2()
{
    initForNewTest(m_loremIpsum);
    QTextCursor cursor(m_doc);
    cursor.insertText("foo\n\n"); // insert empty parag;

    m_layout->layout();
    QTextBlock block = m_doc->begin().next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    QCOMPARE(m_blockLayout->lineCount(), 1);

    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - 28.8) < ROUNDING);
}

void TestDocumentLayout::testAdvancedLineSpacing()
{
    initForNewTest("Line1\nLine2\nLine3\nLine4\nLine5\nLine6\nLine7");
    QTextCursor cursor(m_doc);

    KoParagraphStyle style;
    style.setLineHeightPercent(80);
    QTextBlock block = m_doc->begin();
    style.applyStyle(block);

    // check if styles do their work ;)
    QCOMPARE(block.blockFormat().intProperty(KoParagraphStyle::PercentLineHeight), 80);

    block = block.next();
    QVERIFY(block.isValid()); //line2
    style.setLineHeightAbsolute(28.0); // removes the percentage
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line3
    style.setMinimumLineHeight(40.0);
    style.setLineHeightPercent(120);
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line4
    style.remove(KoParagraphStyle::FixedLineHeight);
    style.setMinimumLineHeight(5.0);
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line5
    style.setMinimumLineHeight(0.0);
    style.setLineSpacing(8.0);
    style.remove(KoParagraphStyle::PercentLineHeight);
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line6
    style.setLineSpacingFromFont(true);
    style.setLineHeightPercent(100);
    style.remove(KoParagraphStyle::LineSpacing);
    style.applyStyle(block);

    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).y(), 0.0);
    block = m_doc->begin().next(); // line2
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (12.0 * 0.8)) < ROUNDING);
    block = block.next(); // line3
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (9.6 + 28.0)) < ROUNDING);
    block = block.next(); // line4
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (37.6 + 40)) < ROUNDING);
    block = block.next(); // line5
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    // qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (77.6 + qMax(12 * 1.2, 5.0))) < ROUNDING); // 92
    block = block.next(); // line6
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QCOMPARE(m_blockLayout->lineAt(0).y(), 92.0 + 12 + 8);

    qreal height = m_blockLayout->lineAt(0).height();
    block = block.next(); // line 7
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QCOMPARE(m_blockLayout->lineAt(0).y(), 112 + height);
}

void TestDocumentLayout::testMargins()
{
    initForNewTest(m_loremIpsum);
    QTextCursor cursor(m_doc);
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setLeftMargin(10.0);
    cursor.setBlockFormat(bf);
    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 10.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 190.0);

    bf.setRightMargin(15.0);
    cursor.setBlockFormat(bf);
    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 10.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 175.0);

    bf.setLeftMargin(0.0);
    cursor.setBlockFormat(bf);
    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 0.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 185.0); // still uses the right margin of 15

    cursor.setPosition(m_loremIpsum.length());
    cursor.insertText("\n");
    bf.setTopMargin(12);
    cursor.setBlockFormat(bf);
    cursor.insertText(m_loremIpsum);// create second parag
    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 0.0); // parag 1
    QCOMPARE(m_blockLayout->lineAt(0).width(), 185.0);

    // and test parag 2
    QTextBlock block2 = m_doc->begin().next();
    QTextLayout *layout = block2.layout();
    QCOMPARE(layout->lineAt(0).x(), 0.0);
    QCOMPARE(layout->lineAt(0).width(), 185.0);

    QTextLine lastLineOfParag1 = m_blockLayout->lineAt(m_blockLayout->lineCount() - 1);
    QTextLine firstLineOfParag2 =  layout->lineAt(0);
    const qreal FONTSIZE = 12.0;
    const qreal BottomParag1 = lastLineOfParag1.y() + (FONTSIZE * 1.2);
    QVERIFY(qAbs(firstLineOfParag2.y() - BottomParag1  - 12.0) < ROUNDING);
}

void TestDocumentLayout::testMultipageMargins()
{
    initForNewTest("123456789\nparagraph 2\nlksdjflksdjflksdjlkfjsdlkfjsdlk sldkfj lsdkjf lskdjf lsd lfsjd lfk");
    QTextCursor cursor(m_doc);

    KoParagraphStyle h1;
    h1.setTopMargin(100.0);
    h1.setBottomMargin(20.0);
    m_styleManager->add(&h1);

    QTextBlock block = m_doc->begin();
    h1.applyStyle(block);
    block = block.next();
    h1.applyStyle(block);

    m_shape1->setSize(QSizeF(200, 14.4 + 100 + 20 + 14.4 + 5)); // 5 for fun..
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(120, 1000));
    m_layout->addShape(shape2);

    m_layout->layout();

    /* The above has 3 parags with a very tall top margin and a bottom margin
     * The first line is expected to be displayed at top of the page because we
     * never show the topMargin on new pages (see also testParagOffset).
     * The second parag will then be 100 + 20 points lower.
     * The shape will not have enough space for the bottom margin of this second parag,
     * but that does not move it to the second shape!
     * The 3th parag is then displayed at the top of the second shape.
     */

    block = m_doc->begin();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // first parag
    QCOMPARE(m_blockLayout->lineAt(0).y(), 0.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // second parag
    //qDebug() << m_blockLayout->lineAt(0).y() << "=" << (12.0 * 1.2 + 100.0 + 20.0);
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (12.0 * 1.2 + 100.0 + 20.0)) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // thirth parag
    // the 10 in the next line is hardcoded distance between frames.
    //qDebug() << m_blockLayout->lineAt(0).y() << "=" << m_shape1->size().height() + FRAME_SPACING;
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (m_shape1->size().height() + FRAME_SPACING)) < ROUNDING);

    /* TODO
        - top margin at new page is honoured when the style used has a
         'page break before' set to true.
     */
}

void TestDocumentLayout::testTextIndent()
{
    initForNewTest(m_loremIpsum);
    QTextCursor cursor(m_doc);
    QTextBlockFormat format = cursor.blockFormat();
    format.setTextIndent(20);
    cursor.setBlockFormat(format);

    m_layout->layout();

    QCOMPARE(m_blockLayout->lineAt(0).x(), 20.0);
    QCOMPARE(m_blockLayout->lineAt(1).x(), 0.0);
}

void TestDocumentLayout::testBasicTextAlignments()
{
    initForNewTest("Left\nCenter\nRight");

    QTextCursor cursor(m_doc);
    QTextBlockFormat format = cursor.blockFormat();
    format.setAlignment(Qt::AlignLeft);
    cursor.setBlockFormat(format);
    cursor.setPosition(6);
    format.setAlignment(Qt::AlignHCenter);
    cursor.setBlockFormat(format);
    cursor.setPosition(13);
    format.setAlignment(Qt::AlignRight);
    cursor.setBlockFormat(format);

    m_layout->layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 0.0);

    QTextBlock block = m_doc->begin().next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();

    QRectF rect = m_blockLayout->lineAt(0).naturalTextRect();
    QVERIFY(rect.x() > 60);
    QCOMPARE(rect.x() * 2 + rect.width(), 200.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout();
    rect = m_blockLayout->lineAt(0).naturalTextRect();
    QVERIFY(rect.x() > 150);
    QCOMPARE(rect.right(), 200.0);
}

void TestDocumentLayout::testTextAlignments()
{
    // TODO justified & justified, last line
    initForNewTest("Left\nRight\nﺵﻻﺆﻴﺜﺒ\nﺵﻻﺆﻴﺜﺒ\nLast Line.");
    KoParagraphStyle start;
    start.setAlignment(Qt::AlignLeading);
    KoParagraphStyle end;
    end.setAlignment(Qt::AlignTrailing);

    KoParagraphStyle startRTL;
    startRTL.setAlignment(Qt::AlignLeading);
    startRTL.setTextProgressionDirection(KoText::RightLeftTopBottom);
    KoParagraphStyle endRTL;
    endRTL.setAlignment(Qt::AlignTrailing);
    endRTL.setTextProgressionDirection(KoText::RightLeftTopBottom);

    QTextBlock block = m_doc->begin();
    start.applyStyle(block);
    block = block.next();
    end.applyStyle(block);
    block = block.next();
    startRTL.applyStyle(block);
    block = block.next();
    endRTL.applyStyle(block);
    block = block.next();
    endRTL.applyStyle(block);

    m_layout->layout();

    // line 'Left'
    QRectF rect = m_blockLayout->lineAt(0).naturalTextRect();
    QCOMPARE(rect.x(), 0.);

    // line 'Right'
    block = m_doc->begin().next();
    rect = block.layout()->lineAt(0).naturalTextRect();
    QCOMPARE(rect.right(), 200.);
    QVERIFY(rect.left() > 0.);

    // line with align Leading and RTL progression
    block = block.next();
    rect = block.layout()->lineAt(0).naturalTextRect();
    QCOMPARE(rect.right(), 200.);
    QVERIFY(rect.left() > 0.); // expect right alignment

    // line with align tailing and RTL progression
    block = block.next();
    rect = block.layout()->lineAt(0).naturalTextRect();
    QCOMPARE(rect.x(), 0.); // expect left alignment

    // non RTL _text_ but RTL progression as well as align trailing
    block = block.next();
    rect = block.layout()->lineAt(0).naturalTextRect();
    QCOMPARE(rect.x(), 0.); // expect left alignment
    // TODO can we check if the dot is the left most painted char?
}

void TestDocumentLayout::testPageBreak()
{
    initForNewTest("line\nParag2\nSimple Parag\nLast");
    KoParagraphStyle style;
    style.setBreakBefore(true);
    style.setBreakAfter(true);
    QTextBlock block = m_doc->begin();
    style.applyStyle(block); // break before Line (ignored) and after, moving Parag2 to a new shape
    block = block.next();
    QVERIFY(block.isValid());
    block = block.next();
    QVERIFY(block.isValid());
    style.setBreakBefore(false); // break after 'simple parag' moving 'Last' to a new shape
    style.setBreakAfter(true);
    style.applyStyle(block);

    m_shape1->setSize(QSizeF(200, 40));
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(200, 100));
    m_layout->addShape(shape2);

    KoShape *shape3 = new MockTextShape();
    shape3->setSize(QSizeF(200, 100));
    m_layout->addShape(shape3);

    m_layout->layout();

    QCOMPARE(m_blockLayout->lineAt(0).y(), 0.0);
    block = m_doc->begin().next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 2
    QCOMPARE(m_blockLayout->lineCount(), 1);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 50.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 3
    //qDebug() << qAbs(m_blockLayout->lineAt(0).y());
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - 64.4) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 4
    QCOMPARE(m_blockLayout->lineCount(), 1);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 160.0);
}

void TestDocumentLayout::testPageBreak2()
{
    initForNewTest("line\nParag2\nSimple Parag\nLast");
    QTextBlock block = m_doc->begin();
    QTextCursor cursor(block);
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
//bf.setPageBreakPolicy(PageBreak_AlwaysBefore);
    cursor.setBlockFormat(bf);
    block = block.next();
    QVERIFY(block.isValid());
    block = block.next();
    QVERIFY(block.isValid());
    cursor = QTextCursor(block);
    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
    cursor.setBlockFormat(bf);

    m_shape1->setSize(QSizeF(200, 40));
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(200, 100));
    m_layout->addShape(shape2);
    KoShape *shape3 = new MockTextShape();
    shape3->setSize(QSizeF(200, 100));
    m_layout->addShape(shape3);

    m_layout->layout();

    QCOMPARE(m_blockLayout->lineAt(0).y(), 0.0);
    block = m_doc->begin().next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 2
    QCOMPARE(m_blockLayout->lineCount(), 1);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 50.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 3
    //qDebug() << qAbs(m_blockLayout->lineAt(0).y());
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - 64.4) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // parag 4
    QCOMPARE(m_blockLayout->lineCount(), 1);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 160.0);
}

void TestDocumentLayout::testParagOffset()
{
    initForNewTest("First line\nSecond line\n");

    /*
      Test for top-of-page indent.
      When text gets moved to the next page the indent above the paragraph should be ignored
      since the moving to the next page is already enough whitespace.
      The user might want to have spacing at the top of the page, though, without using manually
      typed enters.
      There are 2 cases when such spacing is honoured.
      1) When the paragraph has an offset defined in its style as well as a 'break before' property set.
      2) When its in the paragraph-properties, but not in the style (i.e. a manual override)
    */

    KoParagraphStyle h1;
    h1.setTopMargin(20);
    h1.setBreakBefore(true);
    m_styleManager->add(&h1);

    QTextBlock block = m_doc->begin();
    h1.applyStyle(block);
    block = block.next();
    h1.applyStyle(block);

    m_shape1->setSize(QSizeF(200, 100));
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(200, 100));
    m_layout->addShape(shape2);

    // 1)
    m_layout->layout();
    block = m_doc->begin();
    QCOMPARE(m_blockLayout->lineAt(0).y(), 20.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // page 2
    QCOMPARE(m_blockLayout->lineAt(0).y(), 130.0);

    QTextCursor cursor(m_doc->begin());
    QTextBlockFormat bf = cursor.blockFormat();
    cursor.setPosition(0);
    cursor.setPosition(20, QTextCursor::KeepAnchor);
    bf.setTopMargin(30);
    cursor.setBlockFormat(bf);

    // 2)
    m_layout->layout();
    block = m_doc->begin();
    m_blockLayout = block.layout(); // page 1
    QCOMPARE(m_blockLayout->lineAt(0).y(), 30.0);
    block = block.next();
    QVERIFY(block.isValid());
    m_blockLayout = block.layout(); // page 2
    QCOMPARE(m_blockLayout->lineAt(0).y(), 140.0);
}

void TestDocumentLayout::testParagraphBorders()
{
    initForNewTest("Paragraph with Borders\nAnother parag\n");
    QTextCursor cursor(m_doc->begin());
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setProperty(KoParagraphStyle::LeftBorderStyle, KoBorder::BorderSolid);
    bf.setProperty(KoParagraphStyle::TopBorderStyle, KoBorder::BorderSolid);
    bf.setProperty(KoParagraphStyle::BottomBorderStyle, KoBorder::BorderSolid);
    bf.setProperty(KoParagraphStyle::RightBorderStyle, KoBorder::BorderSolid);
    bf.setProperty(KoParagraphStyle::LeftBorderWidth, 8.0);
    bf.setProperty(KoParagraphStyle::TopBorderWidth, 9.0);
    bf.setProperty(KoParagraphStyle::BottomBorderWidth, 10.0);
    bf.setProperty(KoParagraphStyle::RightBorderWidth, 11.0);
    cursor.setBlockFormat(bf);

    m_layout->layout();
    QTextBlock block = m_doc->begin();
    m_blockLayout = block.layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 8.0);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 9.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0);
    block = block.next();
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (9.0 + 14.4 + 10)) < ROUNDING);  // 14.4 is 12 pt font + 20% linespacing

    // borders + padding create the total inset.
    bf.setProperty(KoParagraphStyle::LeftPadding, 5.0);
    bf.setProperty(KoParagraphStyle::RightPadding, 5.0);
    bf.setProperty(KoParagraphStyle::TopPadding, 5.0);
    bf.setProperty(KoParagraphStyle::BottomPadding, 5.0);
    cursor.setBlockFormat(bf);

    m_layout->layout();
    block = m_doc->begin();
    m_blockLayout = block.layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 13.0);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 14.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - 5.0 * 2);
    block = block.next();
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (9.0 + 14.4 + 10 + 5.0 * 2)) < ROUNDING);

    // borders are positioned outside the padding, lets check that to be the case.
    block = m_doc->begin();
    KoTextBlockData *data  = dynamic_cast<KoTextBlockData*>(block.userData());
    QVERIFY(data);
    KoTextBlockBorderData *border = data->border();
    QVERIFY(border);
    QCOMPARE(border->hasBorders(), true);
    QRectF borderOutline = border->rect();
    QCOMPARE(borderOutline.top(), 0.);
    QCOMPARE(borderOutline.left(), 0.);
    QCOMPARE(borderOutline.right(), 200.);

    // qreal borders.  Specify an additional width for each side.
    bf.setProperty(KoParagraphStyle::LeftBorderStyle, KoBorder::BorderDouble);
    bf.setProperty(KoParagraphStyle::TopBorderStyle, KoBorder::BorderDouble);
    bf.setProperty(KoParagraphStyle::BottomBorderStyle, KoBorder::BorderDouble);
    bf.setProperty(KoParagraphStyle::RightBorderStyle, KoBorder::BorderDouble);
    bf.setProperty(KoParagraphStyle::LeftInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::RightInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::BottomInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::TopInnerBorderWidth, 2.0);
    cursor.setBlockFormat(bf);

    m_layout->layout();
    block = m_doc->begin();
    m_blockLayout = block.layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 15.0);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 16.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - (5.0 + 2.0) * 2);
    block = block.next();
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (9.0 + 14.4 + 10 + (5.0 + 2.0) * 2)) < ROUNDING);

    // and last, make the 2 qreal border have a blank space in the middle.
    bf.setProperty(KoParagraphStyle::LeftBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::RightBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::BottomBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::TopBorderSpacing, 3.0);
    cursor.setBlockFormat(bf);

    m_layout->layout();
    block = m_doc->begin();
    m_blockLayout = block.layout();
    QCOMPARE(m_blockLayout->lineAt(0).x(), 18.0);
    QCOMPARE(m_blockLayout->lineAt(0).y(), 19.0);
    QCOMPARE(m_blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - (5.0 + 2.0 + 3.0) * 2);
    block = block.next();
    m_blockLayout = block.layout();
    //qDebug() << m_blockLayout->lineAt(0).y();
    QVERIFY(qAbs(m_blockLayout->lineAt(0).y() - (9.0 + 14.4 + 10 + (5.0 + 2.0 + 3.0) * 2)) < ROUNDING);
}

void TestDocumentLayout::testBorderData()
{
    initForNewTest("Emtpy\nParagraph with Borders\nAnother parag\n");

    KoParagraphStyle style;
    m_styleManager->add(&style);
    style.setTopMargin(10);
    KoListStyle listStyle;
    KoListLevelProperties llp = listStyle.levelProperties(1);
    llp.setStyle(KoListStyle::DecimalItem);
    listStyle.setLevelProperties(llp);
    style.setListStyle(&listStyle);
    style.setLeftBorderWidth(3);

    QTextBlock block = m_doc->begin().next();
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);

    m_layout->layout();

    block = m_doc->begin().next();
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
    QVERIFY(data);
    KoTextBlockBorderData *border = data->border();
    QVERIFY(border);
    // 1st parag is 12 * 120% = 14.4 tall.
    // 2nd parag starts with 10 pt offset = 24.4
    // Hight of border is 2 parags, each 14.4 for text. Plus 10 pt above the 3th parag. = 38.8pt

    // The rules here are
    //  * two paragraphs share a border
    //  * The top indent (of all parags) does not have a border
    //  * The left border is left of the counter
    QCOMPARE(border->rect(), QRectF(0, 24.4, 188, 38.8));

    style.setBottomMargin(5);
    // manually reapply and relayout to force immediate reaction.
    block = m_doc->begin().next();
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);
    m_layout->layout();

    block = m_doc->begin().next();
    border = data->border();
    QVERIFY(border);
    // The tested here is
    //  * the bottom border of the last parag is directly under the text. (so similar to rule 2)
    // This means that the height is the prev 38.8 + the bottom of the top parag: 5pt = 43.8pt
    QCOMPARE(border->rect(), QRectF(0, 24.4, 188, 43.8));

    QCOMPARE(data->counterPosition(), QPointF(3, 24.4));

    block = block.next();
    data = dynamic_cast<KoTextBlockData*>(block.userData());
    QCOMPARE(data->counterPosition(), QPointF(3, 53.8));
}

void TestDocumentLayout::testEmptyParag()
{
    initForNewTest("Foo\n\nBar\n");
    m_layout->layout();
    QTextBlock block = m_doc->begin();
    QTextLayout *lay = block.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    const qreal y = lay->lineAt(0).position().y();

    block = block.next();
    lay = block.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(lay->lineAt(0).position().y() > y);
    QVERIFY(qAbs(lay->lineAt(0).position().y() - 14.4) < ROUNDING);
}

void TestDocumentLayout::testDropCaps()
{
    initForNewTest(QString("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam\nsome more text")); // some not too long text so the dropcap will be bigger than the block

    KoParagraphStyle style;
    style.setDropCaps(false);
    style.setDropCapsLength(1);
    style.setDropCapsLines(4);
    style.setDropCapsDistance(9.0);
    QTextBlock block = m_doc->begin();
    QTextBlock secondblock = block.next();
    style.applyStyle(block);
    
    m_layout->layout();

    // dummy version, caps is still false.
    m_blockLayout = block.layout();
    QVERIFY(m_blockLayout->lineCount() > 2);
    QTextLine line = m_blockLayout->lineAt(0);
    QVERIFY(line.textLength() > 3);

    style.setDropCaps(true);
    style.applyStyle(block);
    m_layout->layout();

    // test that the first text line is the dropcaps and the positions are right.
    QVERIFY(m_blockLayout->lineCount() > 2);
    line = m_blockLayout->lineAt(0);
    QCOMPARE(line.textLength(), 1);

    QCOMPARE(line.position().x(), 0.0);
    QVERIFY(line.position().y() <= 0.0); // can't get a tight-boundingrect here.

    line = m_blockLayout->lineAt(1);
    QVERIFY(line.textLength() > 2);
    qreal heightNormalLine = line.height();
    qreal linexpos = line.position().x();
    QCOMPARE(line.position().y(), 0.0); // aligned top
    QVERIFY(line.position().x() > 20.0); // can't get a tight-boundingrect here.

    // Now test that a following block is moved inward by the same about since
    // it should still be influenced by the dropcap
    m_blockLayout = secondblock.layout();
    QVERIFY(m_blockLayout->lineCount() == 1);
    line = m_blockLayout->lineAt(0);
    QVERIFY(line.textLength() > 3);
    QCOMPARE(line.position().x(), linexpos);
    QVERIFY(line.position().x() > 20.0); // can't get a tight-boundingrect here.

    style.setDropCaps(false); // remove it
    style.applyStyle(block);
    m_layout->layout();
    m_blockLayout = block.layout();

    // test that the first text line is no longer dropcaps
    QVERIFY(m_blockLayout->lineCount() > 2);
    line = m_blockLayout->lineAt(0);
    QVERIFY(line.textLength() > 1);
    QCOMPARE(line.height(), heightNormalLine);
}

void TestDocumentLayout::testNonBreakableLines()
{
    initForNewTest(m_loremIpsum.left(97) + '\n' + m_loremIpsum);
    QTextBlock block = m_doc->begin().next();
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    format.setNonBreakableLines(true);
    cursor.setBlockFormat(format);

    m_shape1->setSize(QSizeF(200, 100));
    KoShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(120, 1000));
    m_layout->addShape(shape2);

    m_layout->layout();

    block = m_doc->begin();
    QTextLayout *l = block.layout();
    // make sure parag1 is in shape 1.
    for (int i = 0; i < l->lineCount(); i++)
        QVERIFY(l->lineAt(i).y() < 100.);

    block = block.next();
    l = block.layout();
    QCOMPARE(l->lineAt(0).y(), 110.);
}

void TestDocumentLayout::testShapePosition()
{
    initForNewTest("line\nParag2\nSimple Parag\nLast");

    m_shape1->setSize(QSizeF(200, 40));
    MockTextShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(200, 100));
    m_layout->addShape(shape2);

    m_layout->layout();

    QCOMPARE(m_shape1->textShapeData()->position(), 0);
    QCOMPARE(m_shape1->textShapeData()->endPosition(), 11);
    QCOMPARE(shape2->textShapeData()->position(), 12);
    QCOMPARE(shape2->textShapeData()->endPosition(), 30);
}

void TestDocumentLayout::testShapePosition2()
{
    initForNewTest("Foo\n" + m_loremIpsum);

    m_shape1->setSize(QSizeF(200, 40));
    MockTextShape *shape2 = new MockTextShape();
    shape2->setSize(QSizeF(200, 1000));
    m_layout->addShape(shape2);

    m_layout->layout();

    QCOMPARE(m_shape1->textShapeData()->position(), 0);
    int split = m_shape1->textShapeData()->endPosition();
    // qDebug() << split;
    QVERIFY(split > 4 && split < m_loremIpsum.length());
    QCOMPARE(shape2->textShapeData()->position(), split + 1);
    QCOMPARE(shape2->textShapeData()->endPosition(), m_loremIpsum.length() + 5);
}

QTEST_KDEMAIN(TestDocumentLayout, GUI)

#include <TestDocumentLayout.moc>
