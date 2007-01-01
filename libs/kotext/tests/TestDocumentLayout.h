#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <QtTest/QtTest>

#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class MockTextShape;
class QTextDocument;
class QTextLayout;

class TestDocumentLayout : public QObject {
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();

    /// Test the hittest of KoTextDocumentLayout
    void testHitTest();

    /// Test breaking lines based on the width of the shape.
    void testLineBreaking();
    /// Test breaking lines for frames with different widths.
    void testMultiFrameLineBreaking();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing2();
    /// Tests advanced linespacing options provided in our style.
    void testAdvancedLineSpacing();

// Block styles
    /// Test top, left, right and bottom margins of paragraphs.
    void testMargins();
    void testMultipageMargins();
    void testTextIndent();
    void testBasicTextAlignments();
    void testTextAlignments();
    void testPageBreak();
    void testPageBreak2();

// Lists
    void testBasicList();
    void testNumberedList();
    void testInterruptedLists(); // consecutiveNumbering
    void testNestedLists();
// relativeBulletSize

    //etc
    void testParagOffset();
    void testParagraphBorders();
    void testBorderData();

private:
    void initForNewTest(const QString &initText = QString());

private:
    QApplication *m_app;

    MockTextShape *shape1;
    QTextDocument *doc;
    KoTextDocumentLayout *layout;
    QTextLayout *blockLayout;
    QString loremIpsum;
    KoStyleManager *styleManager;
};

class MockTextShape : public KoShape {
  public:
    MockTextShape() {
        KoTextShapeData *textShapeData = new KoTextShapeData();
        setUserData(textShapeData);
        layout = new KoTextDocumentLayout(textShapeData->document());
        layout->addShape(this);
        textShapeData->document()->setDocumentLayout(layout);
    }
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    KoTextDocumentLayout *layout;
};

#endif
