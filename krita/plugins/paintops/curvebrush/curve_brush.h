/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _CURVE_BRUSH_H_
#define _CURVE_BRUSH_H_

#include <QVector>
#include <QList>

#include <KoColor.h>

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>
#include <kis_image.h>
#include <kis_painter.h>

class CurveBrush
{

public:
    CurveBrush();
    ~CurveBrush();

    void paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2);

    void setImage(KisImageWSP image) {
        m_image = image;
    }

    void setInkColor(const KoColor &color) {
        m_inkColor = color;
    }

    void setPainter(KisPainter *painter) {
        m_painter = painter;
    }

    void setMinimalDistance(int mininmalDistance) {
        m_minimalDistance = mininmalDistance;
    }

    void setMode(int mode) {
        m_mode = mode;
    }

    void setInterval(int interval) {
        m_interval = interval;
    }

private:

    QPointF getCubicBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, qreal u);
    QPointF getQuadraticBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, qreal u);
    QPointF getLinearBezier(const QPointF &p1, const QPointF &p2, qreal u);

    void addPoint(QPointF p);
    void removeLast();

    void debugColor(const quint8* data);

    void normalizePoint(QPointF &p);
    void rotatePoints(int *x, int *y, qreal centerX, qreal centerY, qreal angle);
    // width and height for interpolation
    KisImageWSP m_image;
    KoColor m_inkColor;

    // temporary device
    KisPaintDeviceSP m_layer;
    KisPaintDeviceSP m_dab;

    KisRandomAccessor * m_readAccessor;
    KisRandomAccessor * m_writeAccessor;
    quint32 m_pixelSize;

    KisPainter * m_painter;

    QPointF m_points[4];
    int m_head;
    int pointCount;

    int m_counter;
    int m_incr;

    int m_interval;
    int m_mode;
    int m_minimalDistance;

};

#endif
