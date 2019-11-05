/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_IMAGE_TESTER_H
#define KIS_IMAGE_TESTER_H

#include <QtTest>

class KisImageTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void layerTests();
    void benchmarkCreation();
    void testBlockLevelOfDetail();
    void testConvertImageColorSpace();
    void testAssignImageProfile();
    void testGlobalSelection();
    void testCloneImage();
    void testLayerComposition();

    void testFlattenLayer();
    void testMergeDown();
    void testMergeDownDestinationInheritsAlpha();
    void testMergeDownDestinationCustomCompositeOp();
    void testMergeDownDestinationSameCompositeOpLayerStyle();
    void testMergeDownDestinationSameCompositeOp();
    void testMergeDownMultipleFrames();

    void testMergeMultiple();
    void testMergeCrossColorSpace();

    void testMergeSelectionMasks();

    void testFlattenImage();

    void testFlattenPassThroughLayer();
    void testMergeTwoPassThroughLayers();

    void testMergePaintOverPassThroughLayer();
    void testMergePassThroughOverPaintLayer();

    void testPaintOverlayMask();
};

#endif

