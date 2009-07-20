#include "TestTableLayout.h"
#include "../TableLayout.h"
#include "../TableData.h"

#include <KoStyleManager.h>
#include <KoTextDocument.h>
#include <KoTableCellStyle.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QPen>

void TestTableLayout::initTestCase()
{
    m_shape = 0;
    m_table = 0;
    m_textLayout = 0;
    m_styleManager = 0;
}

void TestTableLayout::initTest(QTextTableFormat &format, QStringList &cellTexts,
        int rows, int columns)
{
    m_shape = new MockTextShape();
    Q_ASSERT(m_shape);
    m_shape->setSize(QSizeF(200, 1000));

    m_layout = m_shape->layout;
    Q_ASSERT(m_layout);

    m_doc = m_layout->document();
    Q_ASSERT(m_doc);
    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));

    m_textLayout = new Layout(m_layout);
    Q_ASSERT(m_doc);
    m_layout->setLayout(m_textLayout);

    m_styleManager = new KoStyleManager();
    Q_ASSERT(m_styleManager);
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    QTextCursor cursor(m_doc);
    m_table = cursor.insertTable(rows, columns, format);
    Q_ASSERT(m_table);

    // fill the table cells from top left to bottom right with the given texts.
    int position = 0;
    foreach (QString cellText, cellTexts) {
        QTextTableCell cell = m_table->cellAt(position / m_table->rows(),
                position % m_table->columns());
        if (cell.isValid()) {
            cursor = cell.firstCursorPosition();
            cursor.insertText(cellText);
        }
        position++;
    }
}

void TestTableLayout::cleanupTest()
{
    delete m_table;
    delete m_textLayout;
    delete m_styleManager;
}

void TestTableLayout::debugTableLayout(const TableLayout& layout) const
{
    Q_ASSERT(layout.m_table);
    Q_ASSERT(layout.m_tableData);
    qDebug() << qPrintable(QString("<table width=%1 height=%2>").arg(layout.m_tableData->m_width).arg(layout.m_tableData->m_height));
    for (int row = 0; row < layout.m_table->rows(); ++row) {
        qDebug() << " <row>";
        for (int col = 0; col < layout.m_table->columns(); ++col) {
            QTextTableCell cell = layout.m_table->cellAt(row, col);
            qDebug() << qPrintable(QString("  <cell row=%1 col=%2>").arg(row).arg(col));
            qDebug() << qPrintable(QString("   m_columnWidths[%1] = %2, m_columnPositions[%3] = %4").arg(col).arg(layout.m_tableData->m_columnWidths[col]).arg(col).arg(layout.m_tableData->m_columnPositions[col]));
            qDebug() << qPrintable(QString("   m_rowHeights[%1] = %2, m_rowPositions[%3] = %4").arg(row).arg(layout.m_tableData->m_rowHeights[row]).arg(row).arg(layout.m_tableData->m_rowPositions[row]));
            qDebug() << qPrintable(QString("   m_contentHeights[%1][%2] = %3, cellContentRect(cell) = ").arg(row).arg(col).arg(layout.m_tableData->m_contentHeights[row][col])) << layout.cellContentRect(cell);
            QTextFrame::iterator it = cell.begin();
            qDebug() << "   CONTENT {";
            while (!it.atEnd()) {
                if (it.currentFrame()) {
                    qDebug() << "    <frame/>";
                } else {
                    qDebug() << "    <block/>";
                }
                ++it;
            }
            qDebug() << "   }";
            qDebug() << "  </cell>";

        }
        qDebug() << " </row>";
    }
    qDebug() << "</table>";
}

void TestTableLayout::testBoundingRect()
{
    /*
     * TODO:
     * - Test with different borders/margins.
     * - Test with more scenarios/table positions.
     */
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);
    initTest(format, cellTexts);

    // Insert a block before the table.
    QTextCursor textCursor(m_doc);
    textCursor.insertText("Text\n");

    m_layout->layout();

    /*
     * Rule: One single line block before table, so y == 14.4.
     *
     * FIXME!
     */
    //QCOMPARE(m_textLayout->m_tableLayout.boundingRect(), QRectF(0, 14.4, 200, 100));

    cleanupTest();
}

void TestTableLayout::testBasicLayout()
{
    QStringList cellTexts;
    cellTexts.append("Cell 1");
    cellTexts.append("Cell 2");
    cellTexts.append("Cell 3");
    cellTexts.append("Cell 4");
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    /*
     * - 2x2 table
     * - 100pt wide
     * - 2pt table padding.
     * - No table borders or margins.
     */
    initTest(format, cellTexts, 2, 2);

    m_layout->layout();

    // Check that the table and data was correctly added to the table data map.
    QVERIFY(m_textLayout->m_tableLayout.m_tableDataMap.contains(m_table));
    TableData *tableData = m_textLayout->m_tableLayout.m_tableDataMap.value(m_table);
    QVERIFY(tableData);

    // Check table dimensions are correct.
    QCOMPARE(tableData->m_rowPositions.size(), 2);
    QCOMPARE(tableData->m_rowHeights.size(), 2);
    QCOMPARE(tableData->m_columnPositions.size(), 2);
    QCOMPARE(tableData->m_columnWidths.size(), 2);

    // Check cell bounding rectangles.

    /*
     * Cell 0, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 100, 16));

    /*
     * Cell 0, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 0 (no borders/margins/paddings)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(100, 0, 100, 16));

    /*
     * Cell 1, 0 rules:
     *   x = 0 (no borders/margins/paddings)
     *   y = 16 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(0, 16, 100, 16));

    /*
     * Cell 1, 1 rules:
     *   x = 100 (table width/column count)
     *   y = 16 (line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(100, 16, 100, 16));

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
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(2);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    /*
     * - 2x2 table
     * - 100pt wide
     * - 2pt table padding.
     * - No table borders or margins.
     */
    initTest(format, cellTexts, 2, 2);

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
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(2, 2, 100, 16));

    /*
     * Cell 0, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 (table margin)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell2 = m_table->cellAt(0, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell2), QRectF(102, 2, 100, 16));

    /*
     * Cell 1, 0 rules:
     *   x = 2 (only the table margin)
     *   y = 2 + 16 = 18 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell3 = m_table->cellAt(1, 0);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell3), QRectF(2, 18, 100, 16));

    /*
     * Cell 1, 1 rules:
     *   x = 102 (table margin + (table width/column count)))
     *   y = 2 + 16 = 18 (table margin + line height)
     *   width = 200/2 = 100 (table width/column count)
     *   height = 1 * 16 = 16 (number of lines * line height)
     */
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell4), QRectF(102, 18, 100, 16));

    // TODO: Check block positions.

    cleanupTest();
}

void TestTableLayout::testCellStyles()
{
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    /*
     * - 2x2 table
     * - 200x200pt
     * - No table padding.
     * - No table borders.
     */
    initTest(format, cellTexts, 2, 2);

    /*
     * Cell 0, 0
     *
     * Set
     *  inner border = 1pt
     *  outer border = 2pt
     *  border distance = 4pt
     * on all sides.
     */
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QTextTableCellFormat cell1Format;
    cell1Format.setProperty(KoTableCellStyle::TopBorderOuterPen, QPen(Qt::black, 1));
    cell1Format.setProperty(KoTableCellStyle::TopBorderSpacing, 2.0);
    cell1Format.setProperty(KoTableCellStyle::TopBorderInnerPen, QPen(Qt::black, 4));
    cell1Format.setProperty(KoTableCellStyle::LeftBorderOuterPen, QPen(Qt::black, 1));
    cell1Format.setProperty(KoTableCellStyle::LeftBorderSpacing, 2.0);
    cell1Format.setProperty(KoTableCellStyle::LeftBorderInnerPen, QPen(Qt::black, 4));
    cell1Format.setProperty(KoTableCellStyle::BottomBorderOuterPen, QPen(Qt::black, 1));
    cell1Format.setProperty(KoTableCellStyle::BottomBorderSpacing, 2.0);
    cell1Format.setProperty(KoTableCellStyle::BottomBorderInnerPen, QPen(Qt::black, 4));
    cell1Format.setProperty(KoTableCellStyle::RightBorderOuterPen, QPen(Qt::black, 1));
    cell1Format.setProperty(KoTableCellStyle::RightBorderSpacing, 2.0);
    cell1Format.setProperty(KoTableCellStyle::RightBorderInnerPen, QPen(Qt::black, 4));
    cell1Format.setPadding(8.0);
    cell1.setFormat(cell1Format);

    m_layout->layout();

    /*
     * Cell 0, 0 rules:
     *   x = 1 + 2 + 4 = 7
     *   y = 1 + 2 + 4 = 7
     *   width = 100 - (2 * 7)
     *   height = 16 (one line)
     */
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(15, 15, 70, 16));

    /*
     * TODO:
     * - Test actual block positions too.
     * - Fix the compare above once we support min row height.
     */

    cleanupTest();
}

void TestTableLayout::testCellColumnSpanning()
{
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    initTest(format, cellTexts, 2, 2);


    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 1, 2);
    m_layout->layout();
    cell1 = m_table->cellAt(0, 0);

    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 200, 16));

    cleanupTest();
}

void TestTableLayout::testCellRowSpanning()
{
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    initTest(format, cellTexts, 2, 2);

    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 2, 1);

    m_layout->layout();
    cell1 = m_table->cellAt(0, 0);

    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 100, 32));

    cleanupTest();
}

void TestTableLayout::testCellRowAndColumnSpanning()
{
    QStringList cellTexts;
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 300));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    initTest(format, cellTexts, 3, 3);

    QTextTableCell cell1 = m_table->cellAt(0, 0);
    m_table->mergeCells(0, 0, 2, 2);

    m_layout->layout();
    cell1 = m_table->cellAt(0, 0);

    QCOMPARE(m_textLayout->m_tableLayout.cellBoundingRect(cell1), QRectF(0, 0, 200, 32));

    cleanupTest();
}

void TestTableLayout::testCellRowSpanningCellHeight()
{
    QStringList cellTexts;
    // Five blocks in top left cell.
    cellTexts.append("A\nB\nC\nD\nE");
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);

    initTest(format, cellTexts, 2, 2);

    // Let Cell 0, 0 span 2 rows.
    m_table->mergeCells(0, 0, 2, 1);

    m_layout->layout();

    QTextTableCell cell4 = m_table->cellAt(1, 1);

    /*
     * +--------------+--------------+
     * | Height: 73.6 | Height: 16   |
     * |              +--------------+
     * |              | Height:      |
     * |              |  4 * 14.4    |
     * |              |              |
     * |              |              |
     * +--------------+--------------+
     */
    QVERIFY((m_textLayout->m_tableLayout.cellBoundingRect(cell4).height() - (4 * 14.4)) < 0.125);

    cleanupTest();
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
