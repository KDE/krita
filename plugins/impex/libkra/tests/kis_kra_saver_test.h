/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_KRA_SAVER_TEST_H
#define KIS_KRA_SAVER_TEST_H

#include <QtTest>

class KisKraSaverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();

    void testCrashyShapeLayer();

    // XXX: Also test roundtripping of metadata
    void testRoundTrip();

    void testSaveEmpty();
    void testRoundTripFillLayerColor();
    void testRoundTripFillLayerPattern();

    void testRoundTripLayerStyles();

    void testRoundTripAnimation();

    void testRoundTripColorizeMask();

    void testRoundTripShapeLayer();
    void testRoundTripShapeSelection();
    void testRoundTripStoryboard();

    void testExportToReadonly();

};

#endif
