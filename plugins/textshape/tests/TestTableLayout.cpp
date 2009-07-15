#include "TestTableLayout.h"
#include "../TableLayout.h"
#include "../TableData.h"

#include <KoStyleManager.h>
#include <KoTextDocument.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>

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



void TestTableLayout::testConstruction()
{
    QStringList cellTexts;
    QTextTableFormat format;
    initTest(format, cellTexts);

    TableLayout tableLayout1;
    QVERIFY(tableLayout1.table() == 0);

    TableLayout tableLayout2(m_table);
    QVERIFY(tableLayout2.table() == m_table);

    cleanupTest();
}

void TestTableLayout::testSetPosition()
{
    TableLayout tableLayout;
    QPointF position(1, 3);
    tableLayout.setPosition(position);
    QCOMPARE(tableLayout.position(), position);
}

void TestTableLayout::testSetTable()
{
    QStringList cellTexts;
    QTextTableFormat format;
    initTest(format, cellTexts);

    TableLayout tableLayout;
    tableLayout.setTable(m_table);
    QVERIFY(tableLayout.table() == m_table);

    cleanupTest();
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
    format.setHeight(QTextLength(QTextLength::FixedLength, 100));
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
    format.setHeight(QTextLength(QTextLength::FixedLength, 100));
    format.setBorder(0);
    format.setPadding(0);
    format.setMargin(0);
    format.setCellSpacing(0);
    format.setCellPadding(0);
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

    /*
     * TODO:
     *   - Check table cell content rectangles are correct after initial layout.
     *   - Check that block positions are correct after inital layout.
     *   - ... and after new blocks are inserted.
     *   - ... and after new lines are inserted as a result of a line break.
     *   - ... and after table rows / columns are added.
     *   - Check that all the table data internal vectors were resized correctly.
     */

    cleanupTest();
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
