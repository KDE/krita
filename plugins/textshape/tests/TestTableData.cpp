#include "TestTableData.h"
#include "../TableData.h"

#include <QtGlobal>
#include <QRectF>

/**
 * This test case just does basic testing of the TableData class.
 * "Real world" testing of table layout are in TestTableLayout.cpp.
 */

/*
 * This test tests TableData::cellContentRect(int row, int column) in isolation,
 * without using an actual table. It does this by setting up the table data object's
 * private members and testing that correct rectangles are returned.
*/
void TestTableData::testCellContentRect()
{
    const int numTestRows = 10;
    const int numTestCols = 20;
    const qreal testColWidth = 50;
    const qreal testRowHeight = 20;

    TableData tableData;
    tableData.m_columnWidths.resize(numTestCols);
    tableData.m_columnPositions.resize(numTestCols);
    tableData.m_rowHeights.resize(numTestRows);
    tableData.m_rowPositions.resize(numTestRows);

    qreal columnPosition = 0;
    qreal rowPosition = 0;
    int row, col = 0;

    // Set up internal vectors.
    for (row = 0; row < numTestRows; ++row) {
        tableData.m_rowHeights[row] = testRowHeight;
        tableData.m_rowPositions[row] = rowPosition;
        rowPosition += testRowHeight;
    }
    for (col = 0; col < numTestCols; ++col) {
        tableData.m_columnWidths[col] = testColWidth;
        tableData.m_columnPositions[col] = columnPosition;
        columnPosition += testColWidth;
    }
    // Ensure cellContentRect() returns a correct rect on all cells.
    for (row = 0; row < numTestRows; ++row)
        for (col = 0; col < numTestCols; ++col)
            QCOMPARE(tableData.cellContentRect(row, col),
                    QRectF(col * testColWidth, row * testRowHeight, testColWidth, testRowHeight));

    // Change width of column 1 to 12.3.
    tableData.m_columnWidths[1] = 12.3;
    QCOMPARE(tableData.cellContentRect(0, 1), QRectF(testColWidth, 0.0, 12.3, testRowHeight));

    // Change height of row 3 to 22.9.
    tableData.m_rowHeights[3] = 22.9;
    QCOMPARE(tableData.cellContentRect(3, 1), QRectF(testColWidth, testRowHeight*3, 12.3, 22.9));
}

QTEST_MAIN(TestTableData)

#include <TestTableData.moc>
