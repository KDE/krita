/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINT_DEVICE_TESTER_H
#define KIS_PAINT_DEVICE_TESTER_H

#include <QtTest>

class KisPaintDeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testStore();
    void testGeometry();
    void testClear();
    void testCrop();
    void testThumbnail();
    void testThumbnailDeviceWithOffset();
    void testCaching();
    void testRegion();
    void testPixel();
    void testRoundtripReadWrite();
    void testPlanarReadWrite();
    void testRoundtripConversion();
    void testFastBitBlt();
    void testMakeClone();
    void testBltPerformance();
    void testColorSpaceConversion();
    void testDeviceDuplication();
    void testTranslate();
    void testOpacity();
    void testExactBoundsWeirdNullAlphaCase();
    void benchmarkExactBoundsNullDefaultPixel();
    void testAmortizedExactBounds();
    void testNonDefaultPixelArea();
    void testExactBoundsNonTransparent();

    void testReadBytesWrapAround();
    void testWrappedRandomAccessor();
    void testWrappedHLineIterator();
    void testWrappedVLineIterator();
    void testWrappedHLineIteratorReadMoreThanBounds();
    void testWrappedVLineIteratorReadMoreThanBounds();
    void testMoveWrapAround();

    void testCacheState();

    void testLodTransform();
    void testLodDevice();
    void benchmarkLod1Generation();
    void benchmarkLod2Generation();
    void benchmarkLod3Generation();
    void benchmarkLod4Generation();

    void testFramesLeaking();
    void testFramesUndoRedo();
    void testFramesUndoRedoWithChannel();
    void testCrossDeviceFrameCopyDirect();
    void testCrossDeviceFrameCopyChannel();
    void testLazyFrameCreation();
    void testCopyPaintDeviceWithFrames();

    void testCompositionAssociativity();

    void stressTestMemoryFragmentation();
};

#endif

