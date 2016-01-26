/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

