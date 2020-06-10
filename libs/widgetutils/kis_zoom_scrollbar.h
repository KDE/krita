/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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


#ifndef KIS_ZOOM_SCROLLBAR_H
#define KIS_ZOOM_SCROLLBAR_H

#include <QScrollBar>
#include <QVector2D>

#include <kritawidgetutils_export.h>

class KRITAWIDGETUTILS_EXPORT KisZoomableScrollBar : public QScrollBar
{
    Q_OBJECT

private:
    QPoint lastKnownPosition;
    QVector2D accelerationAccumulator;
    qreal scrollSubPixelAccumulator;
    qreal zoomPerpendicularityThreshold;
    bool catchTeleportCorrection = false;

public:
    KisZoomableScrollBar(QWidget* parent = 0);
    KisZoomableScrollBar(Qt::Orientation orientation, QWidget * parent = 0);
    ~KisZoomableScrollBar();

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

Q_SIGNALS:
    void zoom(qreal delta);
    void overscroll(int delta);
};

#endif // KIS_ZOOM_SCROLLBAR_H
