#include "TestTableLayout.h"
#include "../TableLayout.h"
#include "../TableLayoutData.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
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
    Q_ASSERT(m_defaultTableStyle);
    m_defaultTableStyle->setMargin(0.0);
    m_defaultTableStyle->setWidth(QTextLength(QTextLength::FixedLength, 200));
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
    KoParagraphStyle style;
    style.setStyleId(101); // needed to do manually since we don't use the stylemanager
    QTextBlock b2 = m_doc->begin();
    while (b2.isValid()) {
        style.applyStyle(b2);
        b2 = b2.next();
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
    QCOMPARE(tableLayoutData->m_tableRects.last().columnPositions.size(), 2);
    QCOMPARE(tableLayoutData->m_tableRects.last().columnWidths.size(), 2);

    // Check cell bounding rectangles.

    /*
     * Cell 0, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 100, 14.4));

    /*
     * Cell 0, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(100, 0, 100, 14.4));

    /*
     * Cell 1, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 14.4 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(0, 14.4, 100, 14.4));

    /*
     * Cell 1, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 14.4 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(100, 14.4, 100, 14.4));

    // Check position of blocks in cells.

    QTextBlock block1 = cell1.firstCursorPosition().block();
    QCOMPARE(block1.position(), 1);
    QVERIFY(block1.layout());
    QCOMPARE(block1.layout()->lineCount(), 1);
    QTextLine line = block1.layout()->lineAt(0);
    QCOMPARE(line.width(), 100.);
    QCOMPARE(line.position(), QPointF());

    /*
     * Blocks in cell 0,1 rules:
     *   Position      Content      Layout bounding rect
     *   8             "Cell 2"     100, 0 100x?
     */
    QTextBlock block2 = cell2.firstCursorPosition().block();
    QCOMPARE(block2.position(), 8);
    QVERIFY(block2.layout());
    QCOMPARE(block2.layout()->lineCount(), 1);
    line = block2.layout()->lineAt(0);
    QCOMPARE(line.width(), 100.);
    QCOMPARE(line.position(), QPointF(100, 0));

    /*
     * Blocks in cell 1,0 rules:
     *   Position      Content      Layout bounding rect
     *   15            "Cell 3"     0, 14.4 100x?
     */
    QTextBlock block3 = cell3.firstCursorPosition().block();
    QCOMPARE(block3.position(), 15);
    QVERIFY(block3.layout());
    QCOMPARE(block3.layout()->lineCount(), 1);
    line = block3.layout()->lineAt(0);
    QCOMPARE(line.width(), 100.);
    QCOMPARE(line.position().x(), 0.);
    QVERIFY(qAbs(line.position().y() - 14.4) < 0.156);

    /*
     * Blocks in cell 1,1 rules:
     *   Position      Content      Layout bounding rect
     *   22            "Cell 4"     100, 14.4 100x?
     */
    QTextBlock block4 = cell4.firstCursorPosition().block();
    QCOMPARE(block4.position(), 22);
    QVERIFY(block4.layout());
    QCOMPARE(block4.layout()->lineCount(), 1);
    line = block4.layout()->lineAt(0);
    QCOMPARE(line.width(), 100.);
    QCOMPARE(line.position().x(), 100.);
    QVERIFY(qAbs(line.position().y() - 14.4) < 0.156);

    /*
     * TODO: Insert/remove rows/columns.
     */

    cleanupTest();
}

void TestTableLayout::testTableMargin()
{
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
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(2, 2, 100, 14.4));

    /*
     * Cell 0, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 (table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(102, 2, 100, 14.4));

    /*
     * Cell 1, 0 rules:
     *   x = 2 (only the table margin)
     *   y = 2 + 14.4 = 16.4 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(2, 16.4, 100, 14.4));

    /*
     * Cell 1, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 + 14.4 = 16.4 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(102, 16.4, 100, 14.4));

    // TODO: Check block positions.

    cleanupTest();
}

void TestTableLayout::testIndividualTableMargin()
{
    KoTableStyle *tableStyle = new KoTableStyle();
    Q_ASSERT(tableStyle);
    tableStyle->setTopMargin(1.0);
    tableStyle->setRightMargin(2.0);
    tableStyle->setBottomMargin(4.0);
    tableStyle->setLeftMargin(8.0);
    initTestSimple(2, 2, tableStyle);

    m_layout->layout();

    // Check cell bounding rectangles.

    /*
     * Cell 0, 0 rules:
     *   x = 8 (only the table margin)
     *   y = 1 (only the table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(8, 1, 100, 14.4));

    /*
     * Cell 0, 1 rules:
     *   x = 108 (table margin + (table width/column count)))
     *   y = 1 (table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(108, 1, 100, 14.4));

    /*
     * Cell 1, 0 rules:
     *   x = 8 (only the table margin)
     *   y = 1 + 14.4 = 15.4 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(8, 15.4, 100, 14.4));

    /*
     * Cell 1, 1 rules:
     *   x = 108 (table margin + (table width/column count)))
     *   y = 1 + 14.4 = 15.4 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(108, 15.4, 100, 14.4));

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
     *   height = 14.4 (one line)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QTextTableCellFormat cell1Format = cell1.format().toTableCellFormat();
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(15, 15, 70, 14.4));

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
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 200, 14.4));

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
     *   height = 2 * 14.4 = 28.8 (row span 2)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 100, 28.8));

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
     *   height = 2 * 14.4 = 28.8 (row span 2)
     */
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 200, 28.8));

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
     * | C                 | y = 14.4 (1*14.4) |
     * | D                 | w = 100 (200/2)   |
     * | E                 | h = 57.6 (4*14.4) |
     * +-------------------+-------------------+
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    // We have to check the geometries of the rect individually, as there
    // are rounding to consider in the height.
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).x(), 100.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).y(), 14.4);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4).width(), 100.0);
    QVERIFY((m_textLayout->m_tableLayout.cellBoundingRect(cell4).height() - 57.6) < 0.125);

    cleanupTest();
}

void TestTableLayout::testTableWidth()
{
    KoTableStyle *tableStyle = new KoTableStyle();
    QVERIFY(tableStyle);
    initTestSimple(2, 2, tableStyle);

    /*
     * Fixed width.
     *
     * Rules:
     *  - Table should have 1 rect.
     *  - Width of this rect should be 60 pt (fixed).
     */
    tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 60.0));
    QTextTableFormat tableFormat = m_table->format();
    tableStyle->applyStyle(tableFormat);
    m_table->setFormat(tableFormat);
    m_layout->layout();
    QCOMPARE(m_textLayout->m_tableLayout.m_tableLayoutData->m_tableRects.size(), 1);
    QCOMPARE(m_textLayout->m_tableLayout.m_tableLayoutData->m_tableRects.at(0).rect.width(), 60.0);

    /*
     * Relative width:
     *
     * Rules:
     *  - Table should have 1 rect.
     *  - Width of this rect should be 80 pt (40% of shape which is 200).
     */
    tableStyle->setWidth(QTextLength(QTextLength::PercentageLength, 40.0));
    tableStyle->applyStyle(tableFormat);
    m_table->setFormat(tableFormat);
    m_layout->layout();
    QCOMPARE(m_textLayout->m_tableLayout.m_tableLayoutData->m_tableRects.size(), 1);
    QCOMPARE(m_textLayout->m_tableLayout.m_tableLayoutData->m_tableRects.at(0).rect.width(), 80.0);
}

void TestTableLayout::testMinimumRowHeight()
{
    QList<KoTableColumnStyle *> columnStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    // Give row 0 a minimum height of 30 pt.
    QList<KoTableRowStyle *> rowStyles;
    KoTableRowStyle *rowStyle = new KoTableRowStyle();
    QVERIFY(rowStyle);
    rowStyle->setMinimumRowHeight(30.0);
    rowStyles.append(rowStyle);
    // And add four blocks to cell 0,0.
    QMap<QPair<int, int>, QString> cellTexts;
    cellTexts.insert(qMakePair(0, 0), "A\nB\nC\nD");
    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 14.4 + (3 * 14.4) = 59.2 (+- 0.125 for rounding).
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1).x(), 0.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1).y(), 0.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1).width(), 100.0);
    QVERIFY((m_textLayout->m_tableLayout.cellContentRect(cell1).height() - 59.2) < 0.125);

    /*
     * Cell 0,1 rules:
     *   x = 200/2 = 100 (table width/column count)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 14.4 + (3 * 14.4) = 59.2 (+- 0.125 for rounding)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2).x(), 100.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2).y(), 0.0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2).width(), 100.0);
    QVERIFY((m_textLayout->m_tableLayout.cellContentRect(cell2).height() - 59.2) < 0.125);

    // Now remove the four blocks in cell 0,0 and re-layout.
    QTextCursor cursor = cell1.firstCursorPosition();
    cursor.setPosition(cell1.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 30.0 (minimum row height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 100, 30));

    /*
     * Cell 0,1 rules:
     *   x = 200/2 = 100 (table width/column count)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 30.0 (minimum row height)
     */
    cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(100, 0, 100, 30));
}

void TestTableLayout::testVariableColumnWidth()
{
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;
    QList<KoTableColumnStyle *> columnStyles;
    // Give column 0 20% width.
    KoTableColumnStyle *columnStyle1 = new KoTableColumnStyle();
    QVERIFY(columnStyle1);
    columnStyle1->setRelativeColumnWidth(20.0);
    columnStyles.append(columnStyle1);
    // And column 1 80% width.
    KoTableColumnStyle *columnStyle2 = new KoTableColumnStyle();
    QVERIFY(columnStyle2);
    columnStyle2->setRelativeColumnWidth(80.0);
    columnStyles.append(columnStyle2);
    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 40 (20% of width of shape (200 pt))
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 40, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 40 (width of column 0)
     *   y = 0 (no borders/margins/paddings)
     *   width = 160 (80% of width of shape (200 pt))
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(40, 0, 160, 14.4));
}

void TestTableLayout::testColumnWidth()
{
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;
    QList<KoTableColumnStyle *> columnStyles;
    // Give column 0 20 pt width.
    KoTableColumnStyle *columnStyle1 = new KoTableColumnStyle();
    QVERIFY(columnStyle1);
    columnStyle1->setColumnWidth(20.0);
    columnStyles.append(columnStyle1);
    // And column 1 180 pt width.
    KoTableColumnStyle *columnStyle2 = new KoTableColumnStyle();
    columnStyle2->setColumnWidth(180.0);
    QVERIFY(columnStyle2);
    columnStyles.append(columnStyle2);
    initTest(2, 2, 0, columnStyles, rowStyles, cellStyles, cellTexts);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 20 (fixed width)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 20, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 20 (width of column 0)
     *   y = 0 (no borders/margins/paddings)
     *   width = 180 (fixed width)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(20, 0, 180, 14.4));
}

void TestTableLayout::testTableAlignment()
{
    KoTableStyle *tableStyle = new KoTableStyle();
    QVERIFY(tableStyle);
    tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 50.0));
    tableStyle->setAlignment(Qt::AlignRight);
    initTestSimple(1, 2, tableStyle);
    m_layout->layout();

    // Right aligned.

    /*
     * Cell 0,0 rules:
     *   x = 150 = 200 - 50 (shape width - table width)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(150, 0, 25, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 175 = (200 - 50) + 25 ((shape width - table width) + column width)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(175, 0, 25, 14.4));

    // Centered.

    tableStyle->setAlignment(Qt::AlignHCenter);
    QTextTableFormat tableFormat = m_table->format();
    tableStyle->applyStyle(tableFormat);
    m_table->setFormat(tableFormat);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 75 = (200 - 50) / 2 ((shape width - table width) / 2)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(75, 0, 25, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 100 = ((200 - 50) / 2) + 25 (((shape width - table width) / 2) + column width)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(100, 0, 25, 14.4));

    // Left aligned.

    tableStyle->setAlignment(Qt::AlignLeft);
    tableFormat = m_table->format();
    tableStyle->applyStyle(tableFormat);
    m_table->setFormat(tableFormat);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 25, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 50 / 2 = 25 (table width / number of columns)
     *   y = 0 (no borders/margins/paddings)
     *   width = 50 / 2 = 25 (table width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(25, 0, 25, 14.4));

    // Justify aligned.

    tableStyle->setAlignment(Qt::AlignJustify); // Will overrule the explicitly set table width.
    tableFormat = m_table->format();
    tableStyle->applyStyle(tableFormat);
    m_table->setFormat(tableFormat);
    m_layout->layout();

    /*
     * Cell 0,0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200 / 2 = 100 (shape width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 100, 14.4));

    /*
     * Cell 0,1 rules:
     *   x = 200 / 2 = 100 (shape width / number of columns)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200 / 2 = 100 (shape width / number of columns)
     *   height = 1 * 14.4 = 14.4 (number of lines * line height)
     */
    cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(100, 0, 100, 14.4));

    cleanupTest();
}

void TestTableLayout::testFeatureCombination()
{
    /* Let's create a larger 3x3 table with interrelated features
     *  - different borders
     *  - different column widths
     *  - merged cells
     */
    KoTableStyle *tableStyle = new KoTableStyle();
    QVERIFY(tableStyle);
    tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 300.0));
    QList<KoTableRowStyle *> rowStyles;
    QMap<QPair<int, int>, KoTableCellStyle *> cellStyles;
    QMap<QPair<int, int>, QString> cellTexts;


    KoTableCellStyle *cellStyle1 = new KoTableCellStyle();
    QVERIFY(cellStyle1);
    cellStyle1->setPadding(8.0);
    cellStyle1->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 5.0, Qt::black);
    cellStyle1->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 6.0, Qt::black);
    cellStyle1->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 4.0, Qt::black);
    cellStyle1->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 8.0, Qt::black);
    cellStyles.insert(qMakePair(0, 0), cellStyle1);

    KoTableCellStyle *cellStyle2 = new KoTableCellStyle();
    QVERIFY(cellStyle2);
    cellStyle2->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 5.0, Qt::black);
    cellStyle2->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 6.0, Qt::black);
    cellStyle2->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 8.0, Qt::black);
    cellStyle2->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 4.0, Qt::black);
    cellStyles.insert(qMakePair(0, 1), cellStyle2);

    KoTableCellStyle *cellStyle3 = new KoTableCellStyle();
    QVERIFY(cellStyle3);
    cellStyle3->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle3->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle3->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle3->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyles.insert(qMakePair(0, 2), cellStyle3);

    KoTableCellStyle *cellStyle4 = new KoTableCellStyle();
    QVERIFY(cellStyle4);
    cellStyle4->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle4->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle4->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle4->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyles.insert(qMakePair(1, 0), cellStyle4);

    KoTableCellStyle *cellStyle5 = new KoTableCellStyle();
    QVERIFY(cellStyle5);
    cellStyle5->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle5->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle5->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle5->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyles.insert(qMakePair(1, 2), cellStyle5);

    KoTableCellStyle *cellStyle6 = new KoTableCellStyle();
    QVERIFY(cellStyle6);
    cellStyle6->setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle6->setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle6->setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyle6->setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 2.0, Qt::black);
    cellStyles.insert(qMakePair(2, 2), cellStyle6);

    QList<KoTableColumnStyle *> columnStyles;
    // Give column 0 50 pt width.
    KoTableColumnStyle *columnStyle1 = new KoTableColumnStyle();
    QVERIFY(columnStyle1);
    columnStyle1->setColumnWidth(50.0);
    columnStyles.append(columnStyle1);
    // And column 1 100 pt width.
    KoTableColumnStyle *columnStyle2 = new KoTableColumnStyle();
    columnStyle2->setColumnWidth(100.0);
    QVERIFY(columnStyle2);
    columnStyles.append(columnStyle2);
    // And column 2 150 pt width.
    KoTableColumnStyle *columnStyle3 = new KoTableColumnStyle();
    columnStyle3->setColumnWidth(150.0);
    QVERIFY(columnStyle3);
    columnStyles.append(columnStyle3);

    initTest(3, 3, tableStyle, columnStyles, rowStyles, cellStyles, cellTexts);
    m_layout->layout();
    m_table->mergeCells(1, 0, 2, 2);

    /* Top row should be as high as the max required cell height
     * 1st cell requires row to be 12+8+8+14.4=42.4
     * 2nd cell requires row to be 12+14.4=26.4
     * 3rd cell requires row to be 4+14.4=16.4
     *
     * Next row hight depends only on last column 4+14.4
     *
     * Last row hight depends only on last column 4+14.4
     *
     * Thus the merged cell (bottom 2 rows) have a combined height of 36.6 at it's disposal
     *
     * When testing we need to subtract the margins and paddings to get the cellContentRect
     */
    QTextTableCell cell = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(13, 12, 23, 14.4));

    cell = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(55, 8, 89, 30.4));

    cell = m_table->cellAt(0, 2);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(152, 2, 146, 38.4));

    cell = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(2, 28+14.4+2, 146, 4+2*14.4));

    cell = m_table->cellAt(1, 2);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(152, 28+14.4+2, 146, 14.4));

    cell = m_table->cellAt(2, 2);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell), QRectF(152, 32+2*14.4+2, 146, 14.4));

    cleanupTest();
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
