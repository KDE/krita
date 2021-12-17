/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_ZOOM_SCROLLBAR_H
#define KIS_ZOOM_SCROLLBAR_H

#include <QScrollBar>
#include <QVector2D>

#include <kritaui_export.h>

class KRITAUI_EXPORT KisZoomableScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    KisZoomableScrollBar(QWidget* parent = 0);
    KisZoomableScrollBar(Qt::Orientation orientation, QWidget * parent = 0);
    ~KisZoomableScrollBar();

    QPoint barPosition();

    //Catch for teleportation from one side of the screen to the other.
    bool catchTeleports(QMouseEvent* event);

    //Window-space wrapping for mouse dragging. Allows for blender-like
    //infinite mouse scrolls.
    void handleWrap(const QPoint &accel, const QPoint &globalMouseCoord);

    //Scroll based on a mouse acceleration value.
    void handleScroll(const QPoint &accel);

    void tabletEvent(QTabletEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void wheelEvent(QWheelEvent *event) override;

    void setZoomDeadzone(float value);
    void setWheelOverscrollSensitivity(float sensitivity);

Q_SIGNALS:
    void zoom(qreal delta);
    void overscroll(qreal delta);

private:
    QPoint initialPositionRelativeToBar;
    QPoint lastKnownPosition;
    QVector2D accelerationAccumulator;
    qreal scrollSubPixelAccumulator;
    qreal zoomThreshold;
    qreal wheelOverscrollSensitivity;
    bool catchTeleportCorrection = false;
    bool zoomEnabled = true;
};

#endif // KIS_ZOOM_SCROLLBAR_H
