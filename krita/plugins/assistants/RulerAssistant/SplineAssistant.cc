/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "SplineAssistant.h"

#include <kdebug.h>
#include <klocale.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include "kis_coordinates_converter.h"

#include <math.h>
#include <limits>
#include <algorithm>

SplineAssistant::SplineAssistant()
        : KisPaintingAssistant("spline", i18n("Spline assistant"))
{
}

// parametric form of a cubic spline (B(t) = (1-t)^3 P0 + 3 (1-t)^2 t P1 + 3 (1-t) t^2 P2 + t^3 P3)
inline QPointF B(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3)
{
    const qreal
        tp = 1 - t,
        tp2 = tp * tp,
        t2 = t * t;
    return
       (    tp2 * tp) * P0 +
       (3 * tp2 * t ) * P1 +
       (3 * tp  * t2) * P2 +
       (    t   * t2) * P3;
}
// squared distance from a point on the spline to given point: we want to minimize this
inline qreal D(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3, const QPointF& p)
{
    const qreal
        tp = 1 - t,
        tp2 = tp * tp,
        t2 = t * t,
        a =     tp2 * tp,
        b = 3 * tp2 * t,
        c = 3 * tp  * t2,
        d =     t   * t2,
        x_dist = a*P0.x() + b*P1.x() + c*P2.x() + d*P3.x() - p.x(),
        y_dist = a*P0.y() + b*P1.y() + c*P2.y() + d*P3.y() - p.y();
    return x_dist * x_dist + y_dist * y_dist;
}

QPointF SplineAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(handles().size() == 4);
    // minimize d(t), but keep t in the same neighbourhood as before (unless starting a new stroke)
    // (this is a rather inefficient method)
    qreal min_t = std::numeric_limits<qreal>::max();
    qreal d_min_t = std::numeric_limits<qreal>::max();
    for (qreal t = 0; t <= 1; t += 1e-3) {
        qreal d_t = D(t, *handles()[0], *handles()[2], *handles()[3], *handles()[1], pt);
        if (d_t < d_min_t) {
            d_min_t = d_t;
            min_t = t;
        }
    }
    return B(min_t, *handles()[0], *handles()[2], *handles()[3], *handles()[1]);
}

QPointF SplineAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/)
{
    return project(pt);
}

void SplineAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter)
{
    if (handles().size() < 2) return;

    QTransform initialTransform = converter->documentToWidgetTransform();

    QPointF pts[4];
    pts[0] = *handles()[0];
    pts[1] = *handles()[1];
    pts[2] = (handles().size() >= 3) ? (*handles()[2]) : (*handles()[0]);
    pts[3] = (handles().size() >= 4) ? (*handles()[3]) : (handles().size() >= 3) ? (*handles()[2]) : (*handles()[1]);

    gc.setTransform(initialTransform);
    gc.setPen(QColor(0, 0, 0, 75));
    // Draw control lines
    gc.drawLine(pts[0], pts[2]);
    if (handles().size() >= 4) gc.drawLine(pts[1], pts[3]);
    gc.setPen(QColor(0, 0, 0, 125));
    // Draw the spline
    QPainterPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[2], pts[3], pts[1]);
    drawPath(gc, path);
}

QPointF SplineAssistant::buttonPosition() const
{
    return B(0.5, *handles()[0], *handles()[2], *handles()[3], *handles()[1]);
}

SplineAssistantFactory::SplineAssistantFactory()
{
}

SplineAssistantFactory::~SplineAssistantFactory()
{
}

QString SplineAssistantFactory::id() const
{
    return "spline";
}

QString SplineAssistantFactory::name() const
{
    return i18n("Spline");
}

KisPaintingAssistant* SplineAssistantFactory::createPaintingAssistant() const
{
    return new SplineAssistant;
}
