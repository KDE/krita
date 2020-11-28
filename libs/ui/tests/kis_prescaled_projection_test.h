/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPRESCALED_PROJECTION_TEST_H
#define KISPRESCALED_PROJECTION_TEST_H

#include <QtTest>


class KisPrescaledProjection;
class KoZoomHandler;
class QString;

class KisPrescaledProjectionTest : public QObject
{
    Q_OBJECT

private:

    /**
     * test the projection through a normal, but complicated scenario.
     * The prefix is used to save the result qimages and compare them
     * to the prepared correct images.
     */
    bool testProjectionScenario(KisPrescaledProjection & projection, KoZoomHandler * viewConverter, const QString & name);

private Q_SLOTS:

    void testCreation();

    // Doesn't fail yet, but at least writes out several versions
    // of a scaled image. Make them compare with the results when
    // we're done and have everything okay for regressions
    void testScalingUndeferredSmoothingPixelForPixel();

    void testScalingUndeferredSmoothing();

    void benchmarkUpdate();

    void testScrollingZoom100();
    void testScrollingZoom50();
    void testUpdates();

    void testQtScaling();
};

#endif

