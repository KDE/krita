#ifndef TESTTABLELAYOUT_H
#define TESTTABLELAYOUT_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QString>
#include <qtest_kde.h>

#include "../Layout.h"
#include "../TextShape.h"

class MockTextShape;
class KoTextDocumentLayout;
class KoTableStyle;
class KoTableColumnStyle;
class KoTableRowStyle;
class KoTableCellStyle;
class KoStyleManager;
class KoTableColumnAndRowStyleManager;
class QTextDocument;
class QTextTable;

class TestTableLayout : public QObject
{
    Q_OBJECT

public:
    TestTableLayout() {}

private:
    /**
     * Initialize for a new test.
     *
     * @param rows the number of rows in the table.
     * @param columns the number of columns in the table.
     * @param tableStyle the table style to use.
     * @param columnStyles a list of column styles to use.
     * @param rowStyles a list of row styles to use.
     * @param cellStyles a map of cell styles to use, key is QPair<row, col>.
     * @param cellTexts a map of strings to put in the cells, key is QPair<row, col>.
     */
    void initTest(int rows, int columns, KoTableStyle *tableStyle,
            const QList<KoTableColumnStyle *> &columnStyles,
            const QList<KoTableRowStyle *> &rowStyles,
            const QMap<QPair<int, int>, KoTableCellStyle *> &cellStyles,
            const QMap<QPair<int, int>, QString> &cellTexts);

    /**
     * Initialize for a new test. Simplified version.
     *
     * If no arguments are given, the following will be set up:
     *
     * - 2x2 table with no margins and 200 pt fixed width.
     * - Columns get equal width.
     *
     * @param rows the number of rows in the table.
     * @param columns the number of columns in the table.
     * @param tableStyle the table style to use.
     */
    void initTestSimple(int rows = 2, int columns = 2, KoTableStyle *tableStyle = 0);

    /// Clean up after a test.
    void cleanupTest();

private slots:
    /// Common initialization for all tests.
    void init();
    /// Test very basic layout functionality.
    void testBasicLayout();
    /// Test table padding.
    void testTableMargin();
    /// Test table cell styles.
    void testCellStyles();
    /// Test cell column spanning.
    void testCellColumnSpanning();
    /// Test cell row spanning.
    void testCellRowSpanning();
    /// Test cell row and column spanning.
    void testCellRowAndColumnSpanning();
    /*
     * Tests that the height of the right row is updated when the span
     * area of a cell with row spanning gets its content changed from
     * a text insertion.
     */
    void testCellRowSpanningCellHeight();

private:
    QTextDocument *m_doc;
    QTextTable *m_table;
    KoTextDocumentLayout *m_layout;
    KoStyleManager *m_styleManager;
    KoTableColumnAndRowStyleManager *m_tableColumnAndRowStyleManager;
    Layout *m_textLayout;
    MockTextShape *m_shape;

    // Default styles for the test table.
    KoTableStyle *m_defaultTableStyle;
    KoTableColumnStyle *m_defaultColumnStyle;
    KoTableRowStyle *m_defaultRowStyle;
    KoTableCellStyle *m_defaultCellStyle;

    /*
     * This offset variable is used in tests below to compensate
     * for the empty blocks around tables.
     *
     * We should remove it completely once we have proper behavior
     * of blocks around tables, a la Qt.
     */
    static const qreal Y = 14.4;
};

class MockTextShape : public TextShape
{
public:
    MockTextShape() : TextShape(0)
    {
        layout = dynamic_cast<KoTextDocumentLayout*>(textShapeData()->document()->documentLayout());
    }
    void paint(QPainter &painter, const KoViewConverter &converter)
    {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    KoTextDocumentLayout *layout;
};

#endif // TESTTABLELAYOUT_H
