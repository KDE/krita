/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "RulerAssistant.h"

#include <kdebug.h>
#include <klocale.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include "kis_coordinates_converter.h"

#include <math.h>

RulerAssistant::RulerAssistant()
        : KisPaintingAssistant("ruler", i18n("Ruler assistant"))
{
    QList<KisPaintingAssistantHandleSP> handles;
    handles.push_back(new KisPaintingAssistantHandle(10, 10));
    handles.push_back(new KisPaintingAssistantHandle(100, 100));
    initHandles(handles);
}

QPointF RulerAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(handles().size() == 2);
    double x1 = handles()[0]->x();
    double y1 = handles()[0]->y();
    double x2 = handles()[1]->x();
    double y2 = handles()[1]->y();
    double a1 = (y2 - y1) / (x2 - x1);
    double b1 = y1 - x1 * a1;
    double a2 = (x2 - x1) / (y1 - y2);
    double b2 = pt.y() - a2 * pt.x();
    double xm = (b2 - b1) / (a1 - a2);
    return QPointF(xm, xm * a1 + b1);
}

QPointF RulerAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/)
{
    return project(pt);
}

inline double angle(const QPointF& p1, const QPointF& p2)
{
    return atan2(p2.y() - p1.y(), p2.x() - p1.x());
}


inline double norm2(const QPointF& p)
{
    return sqrt(p.x() * p.x() + p.y() * p.y());
}

void RulerAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter)
{
    Q_UNUSED(updateRect);
    if (handles().size() < 2) return;

    QTransform initialTransform = converter->documentToWidgetTransform();

    // Draw the gradient
    QPointF p1 = *handles()[0];
    QPointF p2 = *handles()[1];
    gc.save();
    {
        QTransform gradientTransform = initialTransform;

        gradientTransform.translate(p1.x(), p1.y());
        gradientTransform.rotate(angle(p1, p2) / M_PI * 180);
        gc.setTransform(gradientTransform);

        QLinearGradient gradient(0, -30, 0, 30);
        gradient.setColorAt(0, QColor(0, 0, 0, 0));
        gradient.setColorAt(0.5, QColor(0, 0, 0, 100));
        gradient.setColorAt(1, QColor(0, 0, 0, 0));
        gc.setBrush(gradient);
        gc.setPen(QPen(Qt::NoPen));
        gc.drawRect(QRectF(0, -50, norm2(p2 - p1), 100));
    }
    gc.restore();

    gc.save();
    gc.setTransform(initialTransform);
    gc.setPen(QColor(0, 0, 0, 125));
    gc.drawLine(p1,p2);
    gc.restore();
}

QPointF RulerAssistant::buttonPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

RulerAssistantFactory::RulerAssistantFactory()
{
}

RulerAssistantFactory::~RulerAssistantFactory()
{
}

QString RulerAssistantFactory::id() const
{
    return "ruler";
}

QString RulerAssistantFactory::name() const
{
    return i18n("Ruler");
}

KisPaintingAssistant* RulerAssistantFactory::paintingAssistant(const QRectF& /*imageArea*/) const
{
    return new RulerAssistant;
}
