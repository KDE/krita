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

// NOTE: copied from Liquify Transform Worker cpp file
struct AllPointsFetcherOp
{
    AllPointsFetcherOp(QRectF srcRect) : m_srcRect(srcRect) {}

    inline void processPoint(int col, int row,
                             int prevCol, int prevRow,
                             int colIndex, int rowIndex) {

        Q_UNUSED(prevCol);
        Q_UNUSED(prevRow);
        Q_UNUSED(colIndex);
        Q_UNUSED(rowIndex);

        QPointF pt(col, row);
        m_points << pt;
    }

    inline void nextLine() {
    }

    QVector<QPointF> m_points;
    QRectF m_srcRect;
};


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



void KisGridInterpolationToolsTest::testCalculateCorrectSubGrid_data()
{
    QTest::addColumn<QRect>("originalBoundsForGrid");
    QTest::addColumn<int>("pixelPrecision");
    QTest::addColumn<QRectF>("currentBounds"); // accumulated brush strokes
    QTest::addColumn<QSize>("gridSize");
    QTest::addColumn<QRect>("expected");

    int pixelPrecision = 8;
    QRect originalBounds = QRect(20, 20, 100, 200);
    QTest::addRow("a") << originalBounds << 8 << QRectF(-1000, -1000, 100, 200) << GridIterationTools::calcGridSize(originalBounds, pixelPrecision) << QRect();
    QTest::addRow("real-test-case") << QRect(QPoint(824,30), QSize(393,330)) << 4 << QRectF(QPointF(2519.65,-391.596), QSizeF(385.124, 1269.36)) << QSize(100, 84) << QRect();
    QRect easyBounds = QRect(8, 8, 64+8, 64+8);

    QTest::addRow("top side") << easyBounds << 8 << QRectF(-100, -100, 200, 20) << QSize(8, 8) << QRect();
    QTest::addRow("left side") << easyBounds << 8 << QRectF(-100, -100, 20, 200) << QSize(8, 8) << QRect();
    QTest::addRow("right side") << easyBounds << 8 << QRectF(80, -100, 20, 100) << QSize(8, 8) << QRect();
    QTest::addRow("bottom side") << easyBounds << 8 << QRectF(-100, 80, 100, 20) << QSize(8, 8) << QRect();

    QTest::addRow("tiny change") << QRect(0, 0, 8, 8) << 8 << QRectF(2.1, 2.1, 0.6, 0.6) << QSize(1, 1) << QRect(0, 0, 1, 1);

}

void KisGridInterpolationToolsTest::testCalculateCorrectSubGrid()
{
    QFETCH(QRect, originalBoundsForGrid);
    QFETCH(int, pixelPrecision);
    QFETCH(QRectF, currentBounds);
    QFETCH(QSize, gridSize);
    QFETCH(QRect, expected);

    QRect result = GridIterationTools::calculateCorrectSubGrid(originalBoundsForGrid, pixelPrecision, currentBounds, gridSize);
    if (result != expected) {
        qCritical() << result << expected;// << "for data: " << ppVar(column) << ppVar(row) << ppVar(gridSize);
    }
    QCOMPARE(result, expected);

}

QVector<QPointF> getPoints(QRect srcBounds, int pixelPrecision) {
    AllPointsFetcherOp pointsOp(srcBounds);
    GridIterationTools::processGrid(pointsOp, srcBounds, pixelPrecision);
    return pointsOp.m_points;
}

void KisGridInterpolationToolsTest::testCutOutSubgridFromBounds_data()
{
    QTest::addColumn<QRect>("correctSubgrid");
    QTest::addColumn<QRect>("srcBounds");
    QTest::addColumn<QSize>("gridSize");
    QTest::addColumn<QVector<QPointF>>("originalPoints");
    QTest::addColumn<QList<QRectF>>("expected");

    int pixelPrecision = 8;
    QRect originalBounds = QRect(20, 20, 100, 200);

    QRect srcBounds = QRect(0, 0, 1240, 1754);
    QRect srcBoundsSmall = QRect(0, 0, 100, 100);

    QTest::addRow("out-of-bounds") << QRect(122, 18, 34, 31) << srcBounds << GridIterationTools::calcGridSize(srcBounds, pixelPrecision)
                                   << getPoints(srcBounds, pixelPrecision) << QList<QRectF> {QRectF(0,0, 1239,144), QRectF(0,144, 976,240), QRectF(0,384, 1239,1369)};
    QTest::addRow("out-of-bounds") << QRect(10, 5, 4, 7) << srcBoundsSmall << GridIterationTools::calcGridSize(srcBoundsSmall, pixelPrecision)
                                   << getPoints(srcBoundsSmall, pixelPrecision) << QList<QRectF> {QRectF(0,0, 99,40), QRectF(0,40, 80,48), QRectF(0,88, 99,11)};

    // this happened after an issue in calculateCorrectSubgrid, but it's still weird how it calculates it
    //  GridIterationTools::calcGridSize(QRect(QPoint(824,30), QSize(393,330)), 4)
    QRect srcBounds2 = QRect(QPoint(824,30), QSize(393,330));

    QTest::addRow("out-of-bounds-0-width-case") << QRect(QPoint(100,0),QSize(0,84)) << srcBounds2 << QSize(100, 84)
                                   << getPoints(srcBounds2, 4) << QList<QRectF> {srcBounds2};

}

void KisGridInterpolationToolsTest::testCutOutSubgridFromBounds()
{
    QFETCH(QRect, correctSubgrid);
    QFETCH(QRect, srcBounds);
    QFETCH(QSize, gridSize);
    QFETCH(QVector<QPointF>, originalPoints);
    QFETCH(QList<QRectF>, expected);

    ENTER_FUNCTION() << ppVar(correctSubgrid) << ppVar(srcBounds) << ppVar(gridSize);

    QList<QRectF> result = GridIterationTools::cutOutSubgridFromBounds(correctSubgrid, srcBounds, gridSize, originalPoints);
    if (result != expected) {
        qCritical() << result << expected;// << "for data: " << ppVar(column) << ppVar(row) << ppVar(gridSize);
    }
    QCOMPARE(result, expected);

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
