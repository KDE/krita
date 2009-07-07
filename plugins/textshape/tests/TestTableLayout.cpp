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
    format.setPadding(0);
    format.setMargin(0);
    initTest(format, cellTexts);

    // Insert a block before the table.
    QTextCursor textCursor(m_doc);
    textCursor.insertText("Text");

    m_layout->layout();

    /*
     * Rule: One single line block before table, so y == 14.4.
     */
    QCOMPARE(m_textLayout->m_tableLayout.boundingRect(), QRectF(0, 14.4, 200, 100));

    cleanupTest();
}

void TestTableLayout::testBasicLayout()
{
    QStringList cellTexts;
    cellTexts.append("Cell 1");
    cellTexts.append("Cell\n2");
    cellTexts.append("Cell 3");
    cellTexts.append("Cell 4");
    QTextTableFormat format;
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    format.setWidth(QTextLength(QTextLength::FixedLength, 200));
    format.setHeight(QTextLength(QTextLength::FixedLength, 100));
    format.setPadding(0);
    format.setMargin(0);
    initTest(format, cellTexts, 2, 2);

    // Insert a block before the table.
    QTextCursor textCursor(m_doc);
    textCursor.insertText("Text");

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

    // Check table cell content rectangles are correct.
    QTextTableCell cell1 = m_table->cellAt(0, 0);
    QTextTableCell cell2 = m_table->cellAt(1, 0);
    QTextTableCell cell3 = m_table->cellAt(0, 1);
    QTextTableCell cell4 = m_table->cellAt(1, 1);
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell1), QRectF(0, 0, 100, 50));
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell2), QRectF(0, 50, 100, 50));
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell3), QRectF(100, 0, 100, 50));
    QCOMPARE(m_textLayout->m_tableLayout.cellContentRect(cell4), QRectF(100, 50, 100, 50));

    cleanupTest();
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
