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
    QList<KisPaintingAssistantHandleSP> handles;
    handles.push_back(new KisPaintingAssistantHandle(100, 100));
    handles.push_back(new KisPaintingAssistantHandle(200, 100));
    handles.push_back(new KisPaintingAssistantHandle(100, 200));
    handles.push_back(new KisPaintingAssistantHandle(200, 200));
    initHandles(handles);
}

// parametric form of a cubic spline (B(t) = (1-t)^3 P0 + 3 (1-t)^2 t P1 + 3 (1-t) t^2 P2 + t^3 P3)
inline QPointF B(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3)
{
    const qreal tp = 1 - t;
    return
            (tp * tp * tp) * P0 +
        3 * (tp * tp * t ) * P1 +
        3 * (tp * t  * t ) * P2 +
            (t  * t  * t ) * P3;
}
// squared distance from a point on the spline to given point: we want to minimize this
inline qreal D(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3, const QPointF& p)
{
    const qreal
        tp = 1 - t,
        x_dist = tp * tp * tp * P0.x() +
             3 * tp * tp * t  * P1.x() +
             3 * tp * t  * t  * P2.x() +
                 t  * t  * t  * P3.x() - p.x(),
        y_dist = tp * tp * tp * P0.y() +
             3 * tp * tp * t  * P1.y() +
             3 * tp * t  * t  * P2.y() +
                 t  * t  * t  * P3.y() - p.y();
    return x_dist * x_dist + y_dist * y_dist;
}

QPointF SplineAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(handles().size() == 4);
    // minimize d(t), but keep t in the same neighbourhood as before (unless starting a new stroke)
    // (this is a rather inefficient method)
    qreal min_t, d_min_t = std::numeric_limits<qreal>::max();
    for (qreal t = 0; t <= 1; t += 1e-3) {
        qreal d_t = D(t, *handles()[0], *handles()[1], *handles()[2], *handles()[3], pt);
        if (d_t < d_min_t) {
            d_min_t = d_t;
            min_t = t;
        }
    }
    return B(min_t, *handles()[0], *handles()[1], *handles()[2], *handles()[3]);
}

QPointF SplineAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/) const
{
    return project(pt);
}

void SplineAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter)
{
    Q_UNUSED(updateRect);
    Q_ASSERT(handles().size() == 4);
    QTransform initialTransform = converter->documentToWidgetTransform();

    gc.save();
    gc.setTransform(initialTransform);
    gc.setPen(QColor(0, 0, 0, 75));
    // Draw control lines
    gc.drawLine(*handles()[0], *handles()[1]);
    gc.drawLine(*handles()[2], *handles()[3]);
    gc.setPen(QColor(0, 0, 0, 125));
    // Draw the spline
    QPainterPath path;
    path.moveTo(*handles()[0]);
    path.cubicTo(*handles()[1], *handles()[2], *handles()[3]);
    gc.drawPath(path);
    
    gc.restore();
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

KisPaintingAssistant* SplineAssistantFactory::paintingAssistant(const QRectF& /*imageArea*/) const
{
    return new SplineAssistant;
}
