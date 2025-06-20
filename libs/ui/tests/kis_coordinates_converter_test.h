/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COORDINATES_CONVERTER_TEST_H
#define KIS_COORDINATES_CONVERTER_TEST_H

#include <simpletest.h>

class KisCoordinatesConverterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testConversion();
    void testImageCropping();
    void testTransformations();
    void testConsistency();
    void testRotation();
    void testMirroring();
    void testMirroringCanvasBiggerThanImage();
    void testCanvasOffset();
    void testImageSmallerThanCanvas();
    void testImageResolutionChange();
    void testOffsetLimits();
    void testOffsetLimitsCropping();
    void testZoomMode_data();
    void testZoomMode();

    void testChangeCanvasSize_data();
    void testChangeCanvasSize();

    void testChangeImageResolution_data();
    void testChangeImageResolution();

    void testChangeImageSize_data();
    void testChangeImageSize();

    void testResolutionModes_data();
    void testResolutionModes();
};

#endif /* KIS_COORDINATES_CONVERTER_TEST_H */

