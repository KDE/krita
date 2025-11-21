/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRID_INTERPOLATION_TOOLS_TEST_H
#define KIS_GRID_INTERPOLATION_TOOLS_TEST_H

#include <simpletest.h>

class KisGridInterpolationToolsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCalcGridDimension();
    void testCalcGridSize();
    // void testProcessGrid();
    void testCalculateCellIndexes_data();
    void testCalculateCellIndexes();
    void testPointToIndex_data();
    void testPointToIndex();
    void testPointPolygonIndexToColRow_data();
    void testPointPolygonIndexToColRow();
    void testGetOrthogonalPointApproximation();
    void testCalculateCorrectSubGrid_data();
    void testCalculateCorrectSubGrid();
    void testCutOutSubgridFromBounds_data();
    void testCutOutSubgridFromBounds();
    /*
    void testAdjustAlignedPolygon();
    void testRestoreOriginalPolygonFromAligned();
    void testIterateThroughGrid();

    void testCellOpStruct();
    void testPaintDevicePolygonOpStruct();
    void testQImagePolygonOpStruct();
    void testIncompletePolygonPolicy();
    void testAlwaysCompletePolygonPolicy();
    void testRegularGridIndexesOp();
    */

    void testQImagePolygonOpStructFastAreaCopy();


};

#endif
