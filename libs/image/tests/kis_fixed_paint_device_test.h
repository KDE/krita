/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FIXED_PAINT_DEVICE_TESTER_H
#define KIS_FIXED_PAINT_DEVICE_TESTER_H

#include <simpletest.h>

class KisFixedPaintDeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testSilly();
    void testClear();
    void testFill();
    void testRoundtripQImageConversion();
    void testBltFixed();
    void testBltFixedOpacity();
    void testBltFixedSmall();
    void testColorSpaceConversion();
    void testBltPerformance();
    void testMirroring_data();
    void testMirroring();
};

#endif

