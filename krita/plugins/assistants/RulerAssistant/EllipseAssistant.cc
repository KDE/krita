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

#include "EllipseAssistant.h"

#include <kdebug.h>
#include <klocale.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include "kis_coordinates_converter.h"

#include <math.h>

EllipseAssistant::EllipseAssistant()
        : KisPaintingAssistant("ellipse", i18n("Ellipse assistant")),
        e(QPointF(10, 100), QPointF(110, 100), QPointF(60, 70))
{
    QList<KisPaintingAssistantHandleSP> handles;
    handles.push_back(new KisPaintingAssistantHandle(e.major1()));
    handles.push_back(new KisPaintingAssistantHandle(e.major2()));
    handles.push_back(new KisPaintingAssistantHandle(e.point()));
    initHandles(handles);
}

QPointF EllipseAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(handles().size() == 3);
    e.set(*handles()[0], *handles()[1], *handles()[2]);
    return e.project(pt);
}

QPointF EllipseAssistant::adjustPosition(const QPointF& pt) const
{
    return project(pt);
}

void EllipseAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter)
{
    Q_UNUSED(updateRect);
    Q_ASSERT(handles().size() == 3);
    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        // valid ellipse
        QTransform initialTransform = converter->documentToWidgetTransform();

        gc.save();
        gc.setTransform(initialTransform);
        gc.setTransform(e.getInverse(), true);
        gc.setPen(QColor(0, 0, 0, 125));
        // Draw axes
        gc.drawLine(QPointF(-e.semiMajor(), 0), QPointF(e.semiMajor(), 0));
        gc.drawLine(QPointF(0, -e.semiMinor()), QPointF(0, e.semiMinor()));
        // Draw the ellipse
        gc.drawEllipse(QPointF(0, 0), e.semiMajor(), e.semiMinor());
        
        gc.restore();
    }
}

EllipseAssistantFactory::EllipseAssistantFactory()
{
}

EllipseAssistantFactory::~EllipseAssistantFactory()
{
}

QString EllipseAssistantFactory::id() const
{
    return "ellipse";
}

QString EllipseAssistantFactory::name() const
{
    return i18n("Ellipse");
}

KisPaintingAssistant* EllipseAssistantFactory::paintingAssistant(const QRectF& /*imageArea*/) const
{
    return new EllipseAssistant;
}
