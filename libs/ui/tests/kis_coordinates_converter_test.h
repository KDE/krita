/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COORDINATES_CONVERTER_TEST_H
#define KIS_COORDINATES_CONVERTER_TEST_H

#include <QtTest>

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
};

#endif /* KIS_COORDINATES_CONVERTER_TEST_H */

