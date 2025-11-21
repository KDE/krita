/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGridInterpolationToolsTest.h"

#include <QtMath>
#include <simpletest.h>

#include <kis_grid_interpolation_tools.h>


#include <QRandomGenerator>

#include <KoColor.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <testutil.h>
#include <kis_liquify_transform_worker.h>
#include <kis_algebra_2d.h>


QVector<QVector<int>> getCalcGridDimensionsExpectedValues()
{
    // start, end, pixelPrecision, expected grid size
    return {
        {0, 8, 8, 2},
        {0, 9, 8, 3},
        {8, 16, 8, 2},
        {8, 17, 8, 3},
        {10, 24, 8, 3},
        {10, 25, 8, 4}
    };
}

void KisGridInterpolationToolsTest::testCalcGridDimension()
{

    QVector<QVector<int>> testCases = getCalcGridDimensionsExpectedValues();
    for (int i = 0; i < testCases.length(); i++) {
        QVector<int> test = testCases[i];
        if (GridIterationTools::calcGridDimension(test[0], test[1], test[2]) != test[3]) {
            qCritical() << test;
        }
        QCOMPARE(GridIterationTools::calcGridDimension(test[0], test[1], test[2]), test[3]);
    }

}

void KisGridInterpolationToolsTest::testCalcGridSize()
{

    QVector<QVector<int>> testCases = getCalcGridDimensionsExpectedValues();

    for (int i = 0; i < testCases.length(); i++) {
        for (int j = 0; j < testCases.length(); j++) {
            QVector<int> x = testCases[i];
            QVector<int> y = testCases[j];
            if (x[2] != y[2]) {
                continue; // different pixel precision
            }

            QRect srcBounds = QRect(QPoint(x[0], y[0]), QPoint(x[1], y[1]));
            QSize result = GridIterationTools::calcGridSize(srcBounds, x[2]);
            QCOMPARE(result, QSize(x[3], y[3]));
        }
    }
}


void KisGridInterpolationToolsTest::testCalculateCellIndexes_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("row");
    QTest::addColumn<QSize>("gridSize");
    QTest::addColumn<QVector<int>>("expected");

    QTest::addRow("a") << 0 << 0 << QSize(100, 100) << (QVector<int> {0, 1, 101, 100});
    QTest::addRow("b") << 10 << 0 << QSize(100, 100) << (QVector<int> {10, 11, 111, 110});
    QTest::addRow("c") << 0 << 10 << QSize(100, 100) << (QVector<int> {1000, 1001, 1101, 1100});
    QTest::addRow("d") << 15 << 20 << QSize(100, 100) << (QVector<int> {2015, 2016, 2116, 2115});


    QTest::addRow("e") << 0 << 0 << QSize(100, 200) << (QVector<int> {0, 1, 101, 100});
    QTest::addRow("f") << 10 << 0 << QSize(100, 200) << (QVector<int> {10, 11, 111, 110});
    QTest::addRow("g") << 0 << 10 << QSize(100, 200) << (QVector<int> {1000, 1001, 1101, 1100});
    QTest::addRow("h") << 15 << 20 << QSize(100, 200) << (QVector<int> {2015, 2016, 2116, 2115});

    QTest::addRow("i") << 0 << 0 << QSize(200, 100) << (QVector<int> {0, 1, 201, 200});
    QTest::addRow("j") << 10 << 0 << QSize(200, 100) << (QVector<int> {10, 11, 211, 210});
    QTest::addRow("k") << 0 << 10 << QSize(200, 100) << (QVector<int> {2000, 2001, 2201, 2200});
    QTest::addRow("l") << 15 << 20 << QSize(200, 100) << (QVector<int> {4015, 4016, 4216, 4215});
}

void KisGridInterpolationToolsTest::testCalculateCellIndexes()
{
    // it takes the gridSize and row and column index
    // and then gives out cell indexes (in the long vector with all points)
    // in a clock-wise manner
    // 1 -> 2
    //      |
    //      v
    // 4 <- 3

    QFETCH(int, column);
    QFETCH(int, row);
    QFETCH(QSize, gridSize);
    QFETCH(QVector<int>, expected);

    QVector<int> result = GridIterationTools::calculateCellIndexes(column, row, gridSize);
    if (result != expected) {
        qCritical() << result << expected << "for data: " << ppVar(column) << ppVar(row) << ppVar(gridSize);
    }
    QCOMPARE(result, expected);

}

void KisGridInterpolationToolsTest::testPointToIndex_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("row");
    QTest::addColumn<QSize>("gridSize");
    QTest::addColumn<int>("expected");

    QTest::addRow("a") << 0 << 0 << QSize(100, 100) << 0;
    QTest::addRow("b") << 10 << 0 << QSize(100, 100) << 10;
    QTest::addRow("c") << 0 << 10 << QSize(100, 100) << 1000;
    QTest::addRow("d") << 15 << 20 << QSize(100, 100) << 2015;

    QTest::addRow("e") << 10 << 0 << QSize(100, 200) << 10;
    QTest::addRow("f") << 0 << 10 << QSize(100, 200) << 1000;
    QTest::addRow("g") << 15 << 20 << QSize(100, 200) << 2015;

    QTest::addRow("h") << 10 << 0 << QSize(200, 100) << 10;
    QTest::addRow("i") << 0 << 10 << QSize(200, 100) << 2000;
    QTest::addRow("j") << 15 << 20 << QSize(200, 100) << 4015;
}

void KisGridInterpolationToolsTest::testPointToIndex()
{
    QFETCH(int, column);
    QFETCH(int, row);
    QFETCH(QSize, gridSize);
    QFETCH(int, expected);

    int result = GridIterationTools::pointToIndex(QPoint(column, row), gridSize);
    if (result != expected) {
        qCritical() << result << expected << "for data: " << ppVar(column) << ppVar(row) << ppVar(gridSize);
    }
    QCOMPARE(result, expected);

}

void KisGridInterpolationToolsTest::testPointPolygonIndexToColRow_data()
{
    QTest::addColumn<QPoint>("baseColRow");
    QTest::addColumn<int>("index");
    QTest::addColumn<QPoint>("expected");

    QTest::addRow("a") << QPoint(0, 0) << 0 << QPoint(0, 0);
    QTest::addRow("a") << QPoint(21, 34) << 0 << QPoint(21, 34);
    QTest::addRow("a") << QPoint(21, 34) << 1 << QPoint(22, 34);
    QTest::addRow("a") << QPoint(21, 34) << 2 << QPoint(22, 35);
    QTest::addRow("a") << QPoint(21, 34) << 3 << QPoint(21, 35);


}

void KisGridInterpolationToolsTest::testPointPolygonIndexToColRow()
{
    QFETCH(QPoint, baseColRow);
    QFETCH(int, index);
    QFETCH(QPoint, expected);

    QPoint result = GridIterationTools::Private::pointPolygonIndexToColRow(baseColRow, index);
    if (result != expected) {
        qCritical() << result << expected;// << "for data: " << ppVar(column) << ppVar(row) << ppVar(gridSize);
    }
    QCOMPARE(result, expected);
}


void KisGridInterpolationToolsTest::testGetOrthogonalPointApproximation()
{

}






void KisGridInterpolationToolsTest::testQImagePolygonOpStructFastAreaCopy()
{
    QImage srcImage(TestUtil::fetchDataFileLazy("test_grid_iteration_tools_qimage_fast_area_copy.png"));
    QImage dstImage(srcImage.size(), srcImage.format());

    GridIterationTools::QImagePolygonOp op(srcImage, dstImage, QPointF(), QPointF(-100, 100));
    op.fastCopyArea(QRectF(QPointF(), srcImage.size()));

    //dstImage.save("fast_area_copy_result.png");
}


SIMPLE_TEST_MAIN(KisGridInterpolationToolsTest)
