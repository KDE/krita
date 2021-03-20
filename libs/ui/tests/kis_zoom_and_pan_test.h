/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ZOOM_AND_PAN_TEST_H
#define __KIS_ZOOM_AND_PAN_TEST_H

#include <simpletest.h>

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
