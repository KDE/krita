#include "TestTableLayout.h"
#include "../TableLayout.h"
#include "../TableLayoutData.h"

#include <KoStyleManager.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTextDocument.h>
#include <KoTableCellStyle.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableStyle.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QPoint>
#include <QString>
#include <QPen>

void TestTableLayout::init()
{
    m_doc = 0;
    m_table = 0;
    m_layout = 0;
    m_styleManager = 0;
    m_tableColumnAndRowStyleManager = 0;
    m_textLayout = 0;
    m_shape = 0;
    m_defaultTableStyle = 0;
    m_defaultColumnStyle = 0;
    m_defaultRowStyle = 0;
    m_defaultCellStyle = 0;
}

void TestTableLayout::initTest(int rows, int columns,
        KoTableStyle *tableStyle,
        const QList<KoTableColumnStyle *> &columnStyles,
        const QList<KoTableRowStyle *> &rowStyles,
        const QMap<QPair<int, int>, KoTableCellStyle *> &cellStyles,
        const QMap<QPair<int, int>, QString> &cellTexts)
{
    // Mock shape of size 200x1000 pt.
    m_shape = new MockTextShape();
    Q_ASSERT(m_shape);
    m_shape->setSize(QSizeF(200, 1000));

    // Document layout.
    m_layout = m_shape->layout;
    Q_ASSERT(m_layout);

    // Document.
    m_doc = m_layout->document();
    Q_ASSERT(m_doc);
    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));

    // Layout state (layout helper).
    m_textLayout = new Layout(m_layout);
    Q_ASSERT(m_textLayout);
    m_layout->setLayout(m_textLayout);

    // Style manager.
    m_styleManager = new KoStyleManager();
    Q_ASSERT(m_styleManager);
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    // Table style.
    m_defaultTableStyle = new KoTableStyle();
    m_defaultTableStyle->setMargin(0.0);
    m_defaultTableStyle->setWidth(QTextLength(QTextLength::FixedLength, 200));
    Q_ASSERT(m_defaultTableStyle);
    QTextTableFormat tableFormat;
    if (tableStyle) {
        tableStyle->applyStyle(tableFormat);
    } else {
        m_defaultTableStyle->applyStyle(tableFormat);
    }

    // Column and row style manager.
    m_tableColumnAndRowStyleManager = new KoTableColumnAndRowStyleManager();
    Q_ASSERT(m_tableColumnAndRowStyleManager);
    tableFormat.setProperty(KoTableStyle::ColumnAndRowStyleManager, QVariant::fromValue(reinterpret_cast<void *>(m_tableColumnAndRowStyleManager)));

    // Table.
    QTextCursor cursor(m_doc);
    m_table = cursor.insertTable(rows, columns, tableFormat);
    Q_ASSERT(m_table);

    // Column styles.
    KoTableColumnStyle *m_defaultColumnStyle = new KoTableColumnStyle();
    Q_ASSERT(m_defaultColumnStyle);
    m_defaultColumnStyle->setRelativeColumnWidth(50.0);
    for (int col = 0; col < columns; ++col) {
        if (columnStyles.value(col)) {
            m_tableColumnAndRowStyleManager->setColumnStyle(col, columnStyles.at(col));
        } else {
            m_tableColumnAndRowStyleManager->setColumnStyle(col, m_defaultColumnStyle);
        }
    }

    // Row styles.
    KoTableRowStyle *m_defaultRowStyle = new KoTableRowStyle();
    Q_ASSERT(m_defaultRowStyle);
    for (int row = 0; row < rows; ++row) {
        if (rowStyles.value(row)) {
            m_tableColumnAndRowStyleManager->setRowStyle(row, rowStyles.at(row));
        } else {
            m_tableColumnAndRowStyleManager->setRowStyle(row, m_defaultRowStyle);
        }
    }

    // Cell styles and texts.
    m_defaultCellStyle = new KoTableCellStyle();
    Q_ASSERT(m_defaultCellStyle);
    for (int row = 0; row < m_table->rows(); ++row) {
        for (int col = 0; col < m_table->columns(); ++col) {
            // Style.
            QTextTableCell cell = m_table->cellAt(row, col);
            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
            if (cellStyles.contains(qMakePair(row, col))) {
                cellStyles.value(qMakePair(row, col))->applyStyle(cellFormat);
                cell.setFormat(cellFormat.toCharFormat());
            } else {
                m_defaultCellStyle->applyStyle(cellFormat);
            }
            cell.setFormat(cellFormat.toCharFormat());
            // Text.
            if (cellTexts.contains(qMakePair(row, col))) {
                cell.firstCursorPosition().insertText(cellTexts.value(qMakePair(row, col)));
            }
        }
    }
}

void TestTableLayout::initTestSimple(int rows, int columns, KoTableStyle *tableStyle)
{
    // initTest() will use default ones for these.
    QList<KoTableColumnStyle *> columnStyles;
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;

    initTest(rows, columns, tableStyle, columnStyles, rowStyles, cellStyles, cellTexts);
}

void TestTableLayout::cleanupTest()
{
    delete m_table;
    delete m_styleManager;
    delete m_tableColumnAndRowStyleManager;
    delete m_defaultTableStyle;
    delete m_defaultColumnStyle;
    delete m_defaultRowStyle;
    delete m_defaultCellStyle;
}

void TestTableLayout::testBasicLayout()
{
    QList<KoTableColumnStyle *> columnStyles;
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;
    cellTexts.insert(qMakePair(0, 0), "Cell 1");
    cellTexts.insert(qMakePair(0, 1), "Cell 2");
    cellTexts.insert(qMakePair(1, 0), "Cell 3");
    cellTexts.insert(qMakePair(1, 1), "Cell 4");

    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);

    m_layout->layout();

    // Check that the table and layout data was correctly added to the table data map.
    QVERIFY(m_textLayout->m_tableLayout.m_tableLayoutDataMap.contains(m_table));
    TableLayoutData *tableLayoutData = m_textLayout->m_tableLayout.m_tableLayoutDataMap.value(m_table);
    QVERIFY(tableLayoutData);

    // Check table dimensions are correct.
    QCOMPARE(tableLayoutData->m_rowPositions.size(), 2);
    QCOMPARE(tableLayoutData->m_rowHeights.size(), 2);
    QCOMPARE(tableLayoutData->m_columnPositions.size(), 2);
    QCOMPARE(tableLayoutData->m_columnWidths.size(), 2);

    // Check cell bounding rectangles.

    /*
     * Cell 0, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, Y, 100, 16));

    /*
     * Cell 0, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(100, Y, 100, 16));

    /*
     * Cell 1, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 16 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(0, 16+Y, 100, 16));

    /*
     * Cell 1, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 16 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(100, 16+Y, 100, 16));

    /*
     * TODO:
     *   - Check that block positions are correct after inital layout.
     *   - ... and after new blocks are inserted.
     *   - ... and after new lines are inserted as a result of a line break.
     *   - ... and after table rows / columns are added.
     */

    cleanupTest();
}

void TestTableLayout::testTableMargin() {
    KoTableStyle *tableStyle = new KoTableStyle();
    Q_ASSERT(tableStyle);
    tableStyle->setMargin(2.0);
    initTestSimple(2, 2, tableStyle);

    m_layout->layout();

    // Check cell bounding rectangles.

    /*
     * Cell 0, 0 rules:
     *   x = 2 (only the table margin)
     *   y = 2 (only the table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(2, 2+Y, 100, 16));

    /*
     * Cell 0, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 (table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(102, 2+Y, 100, 16));

    /*
     * Cell 1, 0 rules:
     *   x = 2 (only the table margin)
     *   y = 2 + 16 = 18 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(2, 18+Y, 100, 16));

    /*
     * Cell 1, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 + 16 = 18 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(102, 18+Y, 100, 16));

    // TODO: Check block positions.

    cleanupTest();
}

void TestTableLayout::testCellStyles()
{
    QList<KoTableColumnStyle *> columnStyles;
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    /*
     * Set a style for cell 0, 0:
     * - 8 pt padding.
     * - 7 pt double black border (4 pt inner, 2 pt spacing, 1 pt inner)
     */
    KoTableCellStyle *cell1Style = new KoTableCellStyle();
    Q_ASSERT(cell1Style);
    cell1Style->setPadding(8.0);
    cell1Style->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 7.0, Qt::black);
    cell1Style->setEdgeDoubleBorderValues(KoTableCellStyle::Left, 1.0, 2.0);
    cell1Style->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 7.0, Qt::black);
    cell1Style->setEdgeDoubleBorderValues(KoTableCellStyle::Right, 1.0, 2.0);
    cell1Style->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 7.0, Qt::black);
    cell1Style->setEdgeDoubleBorderValues(KoTableCellStyle::Top, 1.0, 2.0);
    cell1Style->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 7.0, Qt::black);
    cell1Style->setEdgeDoubleBorderValues(KoTableCellStyle::Bottom, 1.0, 2.0);
    cellStyles.insert(qMakePair(0, 0), cell1Style);
    QMap<QPair<int, int>, QString> cellTexts;

    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);

    m_layout->layout();

    /*
     * Cell 0, 0 rules:
     *   x = 1 + 2 + 4 (border) + 8 (padding) = 15
     *   y = 1 + 2 + 4 (border) + 8 (padding) = 15
     *   width = 100 - (2 * 7) (border) - (2 * 8) (padding) = 70
     *   height = 16 (one line)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QTextTableCellFormat cell1Format = cell1.format().toTableCellFormat();
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(15, 15+Y, 70, 16));

    /*
     * TODO:
     * - Test actual block positions too.
     * - Fix the compare above once we support min row height.
     */

    cleanupTest();
}

void TestTableLayout::testCellColumnSpanning()
{
    initTestSimple();

    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 1, 2);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 100*2 = 200 (column span 2)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, Y, 200, 16));

    cleanupTest();
}

void TestTableLayout::testCellRowSpanning()
{
    initTestSimple();

    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 2, 1);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 2 * 16 = 32 (row span 2)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, Y, 100, 32));

    cleanupTest();
}

void TestTableLayout::testCellRowAndColumnSpanning()
{
    KoTableStyle *tableStyle = new KoTableStyle();
    QVERIFY(tableStyle);
    tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 300));

    initTestSimple(3, 3, tableStyle);

    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 2, 2);
    m_layout->layout();
    cell1 = m_table->cellAt(0, 0); // We get it again, just in case.

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 100*2 = 200 (column span 2)
     *   height = 2 * 16 = 32 (row span 2)
     */
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, Y, 200, 32));

    cleanupTest();
}

void TestTableLayout::testCellRowSpanningCellHeight()
{
    QList<KoTableColumnStyle *> columnStyles;
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;
    cellTexts.insert(qMakePair(0, 0), "A\nB\nC\nD\nE");

    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);
    m_table->mergeCells(0, 0, 2, 1);
    m_layout->layout();

    /*
     * Cell 1,1 rules:
     *
     * +-------------------+-------------------+
     * | A                 |                   |
     * |                   +-------------------+
     * | B                 | x = 100 (200/2)   |
     * | C                 | y = 16 (1*16)     |
     * | D                 | w = 100 (200/2)   |
     * | E                 | h = 57.6 (4*14.4) |
     * +-------------------+-------------------+
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    qDebug() << m_textLayout->m_tableLayout.cellBoundingRect(cell4);
    // We have to check the geometries of the rect individually, as there
    // are rounding to consider in the height.
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).x(), 100.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).y(), 16.0+Y);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).width(), 100.0);
    QVERIFY((m_textLayout->m_tableLayout.cellBoundingRect(cell4).height() - 57.6) < 0.125);

    cleanupTest();
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
