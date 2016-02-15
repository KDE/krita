/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ZOOM_AND_PAN_TEST_H
#define __KIS_ZOOM_AND_PAN_TEST_H

#include <QtTest>

class ZoomAndPanTester;


class KisZoomAndPanTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testZoom100ChangingWidgetSize();
    void testZoomOnBorderZoomLevels();

    void testSequentialActionZoomAndPan();
    void testSequentialActionZoomAndPanFullscreen();
    void testSequentialActionZoomAndPanRotate();
    void testSequentialActionZoomAndPanRotateFullscreen();
    void testSequentialActionZoomAndPanMirror();

    void testSequentialWheelZoomAndPan();
    void testSequentialWheelZoomAndPanFullscreen();
    void testSequentialWheelZoomAndPanRotate();
    void testSequentialWheelZoomAndPanRotateFullscreen();
    void testSequentialWheelZoomAndPanMirror();

    void testImageRescaled_0_5();
    void testImageCropped();

    void testRotation_VastScrolling_1_0();
    void testRotation_VastScrolling_0_5();

    void testRotation_NoVastScrolling_1_0();
    void testRotation_NoVastScrolling_0_5();

private:

    bool checkInvariants(const QPointF &baseFlakePoint,
                         const QPoint &oldOffset,
                         const QPointF &oldPreferredCenter,
                         qreal oldZoom,
                         const QPoint &newOffset,
                         const QPointF &newPreferredCenter,
                         qreal newZoom,
                         const QPointF &newTopLeft,
                         const QSize &oldDocumentSize);

    bool checkZoomWithAction(ZoomAndPanTester &t, qreal newZoom, bool limitedZoom = false);
    bool checkZoomWithWheel(ZoomAndPanTester &t, const QPoint &widgetPoint, qreal zoomCoeff, bool limitedZoom = false);
    bool checkPan(ZoomAndPanTester &t, QPoint shift);
    bool checkRotation(ZoomAndPanTester &t, qreal angle);

    void initializeViewport(ZoomAndPanTester &t, bool fullscreenMode, bool rotate, bool mirror);
    void testSequentialActionZoomAndPan(bool fullscreenMode, bool rotate, bool mirror);
    void testSequentialWheelZoomAndPan(bool fullscreenMode, bool rotate, bool mirror);
    void testRotation(qreal vastScrolling, qreal zoom);
};

#endif /* __KIS_ZOOM_AND_PAN_TEST_H */
