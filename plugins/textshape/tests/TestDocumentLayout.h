#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include "../Layout.h"
#include "../TextShape.h"

#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class MockTextShape;
class QTextDocument;
class QTextLayout;

#define ROUNDING 0.126

class TestDocumentLayout : public QObject {
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();

    /// make sure empty paragraphs are initialized properly
    void testEmptyParag();

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
    void testNonBreakableLines();

// Lists
    void testBasicList();
    void testNumberedList();
    void testInterruptedLists(); // consecutiveNumbering
    void testNestedLists();
    void testAutoRestartList();
    void testListParagraphIndent();
    void testRomanNumbering();
    void testUpperAlphaNumbering();
    void testRestartNumbering();
    void testRightToLeftList();
    void testLetterSynchronization();


// relativeBulletSize

    //etc
    void testParagOffset();
    void testParagraphBorders();
    void testBorderData();
    void testDropCaps();

private:
    void initForNewTest(const QString &initText = QString());

private:
    MockTextShape *shape1;
    QTextDocument *doc;
    KoTextDocumentLayout *layout;
    QTextLayout *blockLayout;
    QString loremIpsum;
    KoStyleManager *styleManager;
    Layout *m_textLayout;
};

class MockTextShape : public TextShape {
  public:
    MockTextShape() {
        layout = dynamic_cast<KoTextDocumentLayout*> (textShapeData()->document()->documentLayout());
    }
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    virtual void saveOdf( KoShapeSavingContext & ) const {}
    virtual bool loadOdf( const KoXmlElement &, KoShapeLoadingContext &) { return true; }
    KoTextDocumentLayout *layout;
};

#endif
