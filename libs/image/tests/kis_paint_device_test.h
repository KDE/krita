/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

