#include "TestDocumentLayout.h"

#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"

#include <QtGui>

#include <kdebug.h>
#include <kinstance.h>

#define ROUNDING 0.126
#define FRAME_SPACING 10.0


void TestDocumentLayout::initTestCase() {
    //  To get around this error
    // ASSERT failure in QFontDatabase: "A QApplication object needs to be constructed before FontConfig is used.", file text/qfontdatabase_x11.cpp, line 942
    QCoreApplication::instance()->exit();
    QCoreApplication::instance()->~QCoreApplication();
    QApplication::setQuitOnLastWindowClosed(false);
    m_app = new QApplication(0, 0, false);

    shape1 = 0;
    doc = 0;
    layout = 0;
    blockLayout = 0;

    loremIpsum = QString("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");
}

void TestDocumentLayout::initForNewTest(const QString &initText) {
    // this leaks memory like mad, but who cares ;)
    shape1 = new MockTextShape();
    shape1->resize(QSizeF(200, 1000));

    // this leaks memory like mad, but who cares ;)
    doc = shape1->layout->document();
    Q_ASSERT(doc);
    layout = dynamic_cast<KoTextDocumentLayout*> (doc->documentLayout());
    Q_ASSERT(layout);
    styleManager = new KoStyleManager();
    layout->setStyleManager(styleManager);

    QTextBlock block = doc->begin();
    if(initText.length() > 0) {
        QTextCursor cursor(doc);
        cursor.insertText(initText);
        KoParagraphStyle style;
        style.setStyleId(101); // needed to do manually since we don't use the stylemanager
        QTextBlock b2 = doc->begin();
        while(b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    blockLayout = block.layout();
}

void TestDocumentLayout:: testHitTest() {
    // init a basic document with 3 parags.
    initForNewTest();
    doc->setHtml("<p>lsdjflkdsjf lsdkjf lsdlflksejrl sdflsd flksjdf lksjrpdslfjfsdhtwkr[ivxxmvlwerponldsjf;dslflkjsorindfsn;epsdf</p><p>sldkfnwerpodsnf</p><p>sldkjfnpqwrdsf</p>");
    QTextBlock block = doc->begin();
    double offset=50.0;
    double lineHeight = 0;
    int lines = 0, parag=0;
    double paragOffets[3];
    while(1) {
        if(!block.isValid()) break;
        paragOffets[parag++] = offset;
        QTextLayout *txtLayout = block.layout();
        txtLayout->beginLayout();
        while(1) {
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
    QCOMPARE(layout->hitTest(QPointF(20, 51+lineHeight), Qt::ExactHit), -1);
    QVERIFY(layout->hitTest(QPointF(20, 51+lineHeight), Qt::FuzzyHit) > 0); // line 2

    // parag2
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::ExactHit), 109);
    QCOMPARE(layout->hitTest(QPointF(20, paragOffets[1]), Qt::FuzzyHit), 109);
    QVERIFY(layout->hitTest(QPointF(20, paragOffets[1] + 20), Qt::FuzzyHit) > 109);
}

void TestDocumentLayout::testLineBreaking() {
    initForNewTest(loremIpsum);
    layout->layout();

    //QCOMPARE(blockLayout->lineCount(), 16);
    QCOMPARE(blockLayout->lineForTextPosition(1).width(), 200.0);
}

void TestDocumentLayout::testMultiFrameLineBreaking() {
    initForNewTest(loremIpsum);
    shape1->resize(QSizeF(200, 41)); // fits 3 lines.
    KoShape *shape2 = new MockTextShape();
    shape2->resize(QSizeF(120, 1000));
    layout->addShape(shape2);

    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0);
    QCOMPARE(blockLayout->lineAt(1).width(), 200.0);
    QCOMPARE(blockLayout->lineAt(2).width(), 200.0);
    QCOMPARE(blockLayout->lineAt(3).width(), 120.0);
    QCOMPARE(blockLayout->lineAt(4).width(), 120.0);

    QTextLine line = blockLayout->lineAt(3);
    QVERIFY(line.y() > shape1->size().height()); // it has to be outside of shape1
    const double topOfFrame2 = line.y();
    line = blockLayout->lineAt(4);
    //qDebug() << line.y() - topOfFrame2 - 14.4;
    QVERIFY(qAbs( line.y() - topOfFrame2 - 14.4) < 0.125);
}

void TestDocumentLayout::testBasicLineSpacing() {
    /// Tests incrementing Y pos based on the font size
    initForNewTest(loremIpsum);
    QTextCursor cursor (doc);
    cursor.setPosition(0);
    cursor.setPosition(loremIpsum.length()-1, QTextCursor::KeepAnchor);
    QTextCharFormat charFormat = cursor.charFormat();
    charFormat.setFontPointSize(12);
    cursor.mergeCharFormat(charFormat);
    layout->layout();

    const double fontHeight12 = 12;
    double lineSpacing12 = fontHeight12 * 1.2; // 120% is the normal lineSpacing.
    const double fontHeight18 = 18;
    double lineSpacing18 = fontHeight18 * 1.2; // 120% is the normal lineSpacing.

    //QCOMPARE(blockLayout->lineCount(), 16);
    QCOMPARE(blockLayout->lineForTextPosition(1).width(), 200.0);
    QTextLine line;
    for(int i=0; i < 16; i++) {
        line = blockLayout->lineAt(i);
        // The reason for this weird check is that the values are stored internally
        // as 26.6 fixed point integers. The entire internal text layout is
        // actually done using fixed point arithmetic. This is due to embedded
        // considerations, and offers general performance benefits across all
        // platforms.
        //qDebug() << qAbs(line.y() - i * lineSpacing12);
        QVERIFY(qAbs(line.y() - i * lineSpacing12) < ROUNDING);
    }

    // make first word smaller, should have zero effect on lineSpacing.
    cursor.setPosition(0);
    cursor.setPosition(11, QTextCursor::KeepAnchor);
    charFormat.setFontPointSize(10);
    cursor.mergeCharFormat(charFormat);
    layout->layout();
    for(int i=0; i < 16; i++) {
        line = blockLayout->lineAt(i);
        //qDebug() << i << qAbs(line.y() - i * lineSpacing12);
        QVERIFY(qAbs(line.y() - i * lineSpacing12) < ROUNDING);
    }

    // make first word on second line word bigger, should move that line down a little.
    int pos = blockLayout->lineAt(1).textStart();
    cursor.setPosition(pos);
    cursor.setPosition(pos + 12, QTextCursor::KeepAnchor);
    charFormat.setFontPointSize(18);
    cursor.mergeCharFormat(charFormat);
    layout->layout();
    line = blockLayout->lineAt(0);
    QCOMPARE(line.y(), 0.0);
    line = blockLayout->lineAt(1);
    QVERIFY(qAbs(line.y() - lineSpacing12) < ROUNDING);

    for(int i=2; i < 15; i++) {
        line = blockLayout->lineAt(i);
//qDebug() << "i: " << i << " gives: " << line.y() << " + " <<  line.ascent() << ", " << line.descent() << " = " << line.height();
        QVERIFY(qAbs(line.y() - (lineSpacing12 + lineSpacing18 + (i-2) * lineSpacing12)) < ROUNDING);
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
    mw.setCentralWidget(new Widget(layout));
    mw.show();
    m_app->exec(); */
}

void TestDocumentLayout::testBasicLineSpacing2() {
    initForNewTest(loremIpsum);
    QTextCursor cursor (doc);
    cursor.insertText("foo\n\n"); // insert empty parag;

    layout->layout();
    QTextBlock block = doc->begin().next();
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    QCOMPARE(blockLayout->lineCount(), 1);

    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - 28.8) < ROUNDING);
}

void TestDocumentLayout::testAdvancedLineSpacing() {
    initForNewTest("Line1\nLine2\nLine3\nLine4\nLine5\nLine6\nLine7");
    QTextCursor cursor(doc);

    KoParagraphStyle style;
    style.setLineHeightPercent(80);
    QTextBlock block = doc->begin();
    style.applyStyle(block);

    // check if styles do their work ;)
    QCOMPARE(block.blockFormat().intProperty(KoParagraphStyle::FixedLineHeight), 80);

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
    style.setMinimumLineHeight(5.0);
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line5
    style.setMinimumLineHeight(0.0);
    style.setLineSpacing(8.0); // implicitly will ignore the other two
    style.applyStyle(block);

    block = block.next();
    QVERIFY(block.isValid()); // line6
    style.setLineSpacingFromFont(true);
    style.setLineHeightPercent(100);
    style.applyStyle(block);

    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).y(), 0.0);
    block = doc->begin().next(); // line2
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - (12.0 * 0.8)) < ROUNDING);
    block = block.next(); // line3
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - (9.6 + 28.0)) < ROUNDING);
    block = block.next(); // line4
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - (37.6 + 40)) < ROUNDING);
    block = block.next(); // line5
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - (77.6 + qMax(12 * 1.2, 5.0))) < ROUNDING);
    block = block.next(); // line6
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QCOMPARE(blockLayout->lineAt(0).y(), 92.0 + 12 + 8);

    double height = blockLayout->lineAt(0).height();
    block = block.next(); // line 7
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QCOMPARE(blockLayout->lineAt(0).y(), 112 + height);
}

void TestDocumentLayout::testMargins() {
    initForNewTest(loremIpsum);
    QTextCursor cursor(doc);
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setLeftMargin(10.0);
    cursor.setBlockFormat(bf);
    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 10.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 190.0);

    bf.setRightMargin(15.0);
    cursor.setBlockFormat(bf);
    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 10.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 175.0);

    bf.setLeftMargin(0.0);
    cursor.setBlockFormat(bf);
    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 185.0); // still uses the right margin of 15

    cursor.setPosition(loremIpsum.length());
    cursor.insertText("\n");
    bf.setTopMargin(12);
    cursor.setBlockFormat(bf);
    cursor.insertText(loremIpsum);// create second parag
    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 0.0); // parag 1
    QCOMPARE(blockLayout->lineAt(0).width(), 185.0);

    // and test parag 2
    QTextBlock block2 = doc->begin().next();
    QTextLayout *layout = block2.layout();
    QCOMPARE(layout->lineAt(0).x(), 0.0);
    QCOMPARE(layout->lineAt(0).width(), 185.0);

    QTextLine lastLineOfParag1 =  blockLayout->lineAt(blockLayout->lineCount()-1);
    QTextLine firstLineOfParag2 =  layout->lineAt(0);
    const double FONTSIZE = 12.0;
    const double BottomParag1 = lastLineOfParag1.y() + (FONTSIZE * 1.2);
    QVERIFY(qAbs(firstLineOfParag2.y() - BottomParag1  - 12.0) < ROUNDING);
}

void TestDocumentLayout::testMultipageMargins() {
    initForNewTest("123456789\nparagraph 2\nlksdjflksdjflksdjlkfjsdlkfjsdlk sldkfj lsdkjf lskdjf lsd lfsjd lfk");
    QTextCursor cursor(doc);

    KoParagraphStyle h1;
    h1.setTopMargin(100.0);
    h1.setBottomMargin(20.0);
    styleManager->add(&h1);

    QTextBlock block = doc->begin();
    h1.applyStyle(block);
    block = block.next();
    h1.applyStyle(block);

    shape1->resize(QSizeF(200, 14.4 + 100 + 20 + 12 + 5)); // 5 for fun..
    KoShape *shape2 = new MockTextShape();
    shape2->resize(QSizeF(120, 1000));
    layout->addShape(shape2);

    layout->layout();

    /* The above has 3 parags with a very tall top margin and a bottom margin
     * The first line is expected to be displayed at top of the page because we
     * never show the topMargin on new pages (see also testParagOffset).
     * The second parag will then be 100 + 20 points lower.
     * The shape will not have enough space for the bottom margin of this second parag,
     * but that does not move it to the second shape!
     * The 3th parag is then displayed at the top of the second shape.
     */

    block = doc->begin();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // first parag
    QCOMPARE(blockLayout->lineAt(0).y(), 0.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // second parag
    //qDebug() << blockLayout->lineAt(0).y() << "=" << (12.0 * 1.2 + 100.0 + 20.0);
    QVERIFY( qAbs(blockLayout->lineAt(0).y() - (12.0 * 1.2 + 100.0 + 20.0)) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // thirth parag
    // the 10 in the next line is hardcoded distance between frames.
    //qDebug() << blockLayout->lineAt(0).y() << "=" << shape1->size().height() + FRAME_SPACING;
    QVERIFY( qAbs(blockLayout->lineAt(0).y() - (shape1->size().height() + FRAME_SPACING)) < ROUNDING);

    /* TODO
        - top margin at new page is honoured when the style used has a
         'page break before' set to true.
     */
}

void TestDocumentLayout::testTextIndent() {
    initForNewTest(loremIpsum);
    QTextCursor cursor(doc);
    QTextBlockFormat format = cursor.blockFormat();
    format.setTextIndent(20);
    cursor.setBlockFormat(format);

    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).x(), 20.0);
    QCOMPARE(blockLayout->lineAt(1).x(), 0.0);
}

void TestDocumentLayout::testBasicTextAlignments() {
    initForNewTest("Left\nCenter\nRight");

    QTextCursor cursor(doc);
    QTextBlockFormat format = cursor.blockFormat();
    format.setAlignment(Qt::AlignLeft);
    cursor.setBlockFormat(format);
    cursor.setPosition(6);
    format.setAlignment(Qt::AlignHCenter);
    cursor.setBlockFormat(format);
    cursor.setPosition(13);
    format.setAlignment(Qt::AlignRight);
    cursor.setBlockFormat(format);

    layout->layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 0.0);

    QTextBlock block = doc->begin().next();
    QVERIFY(block.isValid());
    blockLayout = block.layout();

    QRectF rect = blockLayout->lineAt(0).naturalTextRect();
    QVERIFY(rect.x() > 60);
    QCOMPARE(rect.x() * 2 + rect.width(), 200.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout();
    rect = blockLayout->lineAt(0).naturalTextRect();
    QVERIFY(rect.x() > 150);
    QCOMPARE(rect.right(), 200.0);
}

void TestDocumentLayout::testTextAlignments() {
    // TODO
    // justified
    // justified, last line
    // RTL text

    initForNewTest(loremIpsum);
    KoParagraphStyle style;
    style.setAlignment(Qt::AlignJustify);
    QTextBlock block = doc->begin();
    style.applyStyle(block);
}

void TestDocumentLayout::testPageBreak() {
    initForNewTest("line\nParag2\nSimple Parag\nLast");
    KoParagraphStyle style;
    style.setBreakBefore(true);
    style.setBreakAfter(true);
    QTextBlock block = doc->begin();
    style.applyStyle(block); // break before Line (ignored) and after, moving Parag2 to a new shape
    block = block.next();
    QVERIFY(block.isValid());
    block = block.next();
    QVERIFY(block.isValid());
    style.setBreakBefore(false); // break after 'simple parag' moving 'Last' to a new shape
    style.setBreakAfter(true);
    style.applyStyle(block);

    shape1->resize(QSizeF(200, 40));
    KoShape *shape2 = new MockTextShape();
    shape2->resize(QSizeF(200, 100));
    layout->addShape(shape2);

    KoShape *shape3 = new MockTextShape();
    shape3->resize(QSizeF(200, 100));
    layout->addShape(shape3);

    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).y(), 0.0);
    block = doc->begin().next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 2
    QCOMPARE(blockLayout->lineCount(), 1);
    QCOMPARE(blockLayout->lineAt(0).y(), 50.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 3
    //qDebug() << qAbs(blockLayout->lineAt(0).y());
    QVERIFY( qAbs(blockLayout->lineAt(0).y() - 64.4) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 4
    QCOMPARE(blockLayout->lineCount(), 1);
    QCOMPARE(blockLayout->lineAt(0).y(), 160.0);
}

void TestDocumentLayout::testPageBreak2() {
    initForNewTest("line\nParag2\nSimple Parag\nLast");
    QTextBlock block = doc->begin();
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

    shape1->resize(QSizeF(200, 40));
    KoShape *shape2 = new MockTextShape();
    shape2->resize(QSizeF(200, 100));
    layout->addShape(shape2);
    KoShape *shape3 = new MockTextShape();
    shape3->resize(QSizeF(200, 100));
    layout->addShape(shape3);

    layout->layout();

    QCOMPARE(blockLayout->lineAt(0).y(), 0.0);
    block = doc->begin().next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 2
    QCOMPARE(blockLayout->lineCount(), 1);
    QCOMPARE(blockLayout->lineAt(0).y(), 50.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 3
    //qDebug() << qAbs(blockLayout->lineAt(0).y());
    QVERIFY( qAbs(blockLayout->lineAt(0).y() - 64.4) < ROUNDING);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // parag 4
    QCOMPARE(blockLayout->lineCount(), 1);
    QCOMPARE(blockLayout->lineAt(0).y(), 160.0);
}

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

void TestDocumentLayout::testParagOffset() {
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
    styleManager->add(&h1);

    QTextBlock block = doc->begin();
    h1.applyStyle(block);
    block = block.next();
    h1.applyStyle(block);

    shape1->resize(QSizeF(200, 100));
    KoShape *shape2 = new MockTextShape();
    shape2->resize(QSizeF(200, 100));
    layout->addShape(shape2);

    // 1)
    layout->layout();
    block = doc->begin();
    QCOMPARE(blockLayout->lineAt(0).y(), 20.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // page 2
    QCOMPARE(blockLayout->lineAt(0).y(), 130.0);

    QTextCursor cursor(doc->begin());
    QTextBlockFormat bf = cursor.blockFormat();
    cursor.setPosition(0);
    cursor.setPosition(20, QTextCursor::KeepAnchor);
    bf.setTopMargin(30);
    cursor.setBlockFormat(bf);

    // 2)
    layout->layout();
    block = doc->begin();
    blockLayout = block.layout(); // page 1
    QCOMPARE(blockLayout->lineAt(0).y(), 30.0);
    block = block.next();
    QVERIFY(block.isValid());
    blockLayout = block.layout(); // page 2
    QCOMPARE(blockLayout->lineAt(0).y(), 140.0);
}

void TestDocumentLayout::testParagraphBorders() {
    initForNewTest("Paragraph with Borders\nAnother parag\n");
    QTextCursor cursor(doc->begin());
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setProperty(KoParagraphStyle::LeftBorderStyle, KoParagraphStyle::BorderSolid);
    bf.setProperty(KoParagraphStyle::TopBorderStyle, KoParagraphStyle::BorderSolid);
    bf.setProperty(KoParagraphStyle::BottomBorderStyle, KoParagraphStyle::BorderSolid);
    bf.setProperty(KoParagraphStyle::RightBorderStyle, KoParagraphStyle::BorderSolid);
    bf.setProperty(KoParagraphStyle::LeftBorderWidth, 8.0);
    bf.setProperty(KoParagraphStyle::TopBorderWidth, 9.0);
    bf.setProperty(KoParagraphStyle::BottomBorderWidth, 10.0);
    bf.setProperty(KoParagraphStyle::RightBorderWidth, 11.0);
    cursor.setBlockFormat(bf);

    layout->layout();
    QTextBlock block = doc->begin();
    blockLayout = block.layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 8.0);
    QCOMPARE(blockLayout->lineAt(0).y(), 9.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0);
    block = block.next();
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - ( 9.0 + 14.4 + 10)) < ROUNDING); // 14.4 is 12 pt font + 20% linespacing

    // borders + padding create the total inset.
    bf.setProperty(KoParagraphStyle::LeftPadding, 5.0);
    bf.setProperty(KoParagraphStyle::RightPadding, 5.0);
    bf.setProperty(KoParagraphStyle::TopPadding, 5.0);
    bf.setProperty(KoParagraphStyle::BottomPadding, 5.0);
    cursor.setBlockFormat(bf);

    layout->layout();
    block = doc->begin();
    blockLayout = block.layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 13.0);
    QCOMPARE(blockLayout->lineAt(0).y(), 14.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - 5.0 * 2);
    block = block.next();
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - ( 9.0 + 14.4 + 10 + 5.0 * 2)) < ROUNDING);

    // double borders.  Specify an additional width for each side.
    bf.setProperty(KoParagraphStyle::LeftBorderStyle, KoParagraphStyle::BorderDouble);
    bf.setProperty(KoParagraphStyle::TopBorderStyle, KoParagraphStyle::BorderDouble);
    bf.setProperty(KoParagraphStyle::BottomBorderStyle, KoParagraphStyle::BorderDouble);
    bf.setProperty(KoParagraphStyle::RightBorderStyle, KoParagraphStyle::BorderDouble);
    bf.setProperty(KoParagraphStyle::LeftInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::RightInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::BottomInnerBorderWidth, 2.0);
    bf.setProperty(KoParagraphStyle::TopInnerBorderWidth, 2.0);
    cursor.setBlockFormat(bf);

    layout->layout();
    block = doc->begin();
    blockLayout = block.layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 15.0);
    QCOMPARE(blockLayout->lineAt(0).y(), 16.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - (5.0 + 2.0) * 2);
    block = block.next();
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - ( 9.0 + 14.4 + 10 + (5.0 + 2.0) * 2)) < ROUNDING);

    // and last, make the 2 double border have a blank space in the middle.
    bf.setProperty(KoParagraphStyle::LeftBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::RightBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::BottomBorderSpacing, 3.0);
    bf.setProperty(KoParagraphStyle::TopBorderSpacing, 3.0);
    cursor.setBlockFormat(bf);

    layout->layout();
    block = doc->begin();
    blockLayout = block.layout();
    QCOMPARE(blockLayout->lineAt(0).x(), 18.0);
    QCOMPARE(blockLayout->lineAt(0).y(), 19.0);
    QCOMPARE(blockLayout->lineAt(0).width(), 200.0 - 8.0 - 11.0 - (5.0 + 2.0 + 3.0) * 2);
    block = block.next();
    blockLayout = block.layout();
    //qDebug() << blockLayout->lineAt(0).y();
    QVERIFY(qAbs(blockLayout->lineAt(0).y() - ( 9.0 + 14.4 + 10 + (5.0 + 2.0 + 3.0) * 2)) < ROUNDING);
}

void TestDocumentLayout::testBorderData() {
    initForNewTest("Emtpy\nParagraph with Borders\nAnother parag\n");

    KoParagraphStyle style;
    style.setTopMargin(10);
    KoListStyle listStyle;
    listStyle.setStyle(KoListStyle::DecimalItem);
    style.setListStyle(listStyle);
    style.setLeftBorderWidth(3);

    QTextBlock block = doc->begin().next();
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);

    layout->layout();

    block = doc->begin().next();
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
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
    QCOMPARE(border->rect(), QRectF(0, 24.4, 200, 38.8));

    style.setBottomMargin(5);
    // manually reapply and relayout to force immediate reaction.
    block = doc->begin().next();
    style.applyStyle(block);
    block = block.next();
    style.applyStyle(block);
    layout->layout();

    block = doc->begin().next();
    border = data->border();
    QVERIFY(border);
    // The tested here is
    //  * the bottom border of the last parag is directly under the text. (so similar to rule 2)
    // This means that the height is the prev 38.8 + the bottom of the top parag: 5pt = 43.8pt
    QCOMPARE(border->rect(), QRectF(0, 24.4, 200, 43.8));

    QCOMPARE(data->counterPosition(), QPointF(3, 24.4));

    block = block.next();
    data = dynamic_cast<KoTextBlockData*> (block.userData());
    QCOMPARE(data->counterPosition(), QPointF(3, 53.8));
}

void TestDocumentLayout::testEmptyParag() {
    initForNewTest("Foo\n\nBar\n");
    layout->layout();
    QTextBlock block = doc->begin();
    QTextLayout *lay = block.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    const double y = lay->lineAt(0).position().y();

    block = block.next();
    lay = block.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(lay->lineAt(0).position().y() > y);
    QVERIFY(qAbs(lay->lineAt(0).position().y() - 14.4) < ROUNDING);
}

QTEST_MAIN(TestDocumentLayout)
#include "TestDocumentLayout.moc"
