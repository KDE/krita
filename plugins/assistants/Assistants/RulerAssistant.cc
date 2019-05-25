/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "RulerAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>

#include <math.h>

RulerAssistant::RulerAssistant()
        : KisPaintingAssistant("ruler", i18n("Ruler assistant"))
{
}

KisPaintingAssistantSP RulerAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new RulerAssistant(*this, handleMap));
}

RulerAssistant::RulerAssistant(const RulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
{
}

QPointF RulerAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(isAssistantComplete());
    QPointF pt1 = *handles()[0];
    QPointF pt2 = *handles()[1];
    
    QPointF a = pt - pt1;
    QPointF u = pt2 - pt1;
    
    qreal u_norm = sqrt(u.x() * u.x() + u.y() * u.y());
    
    if(u_norm == 0) return pt;
    
    u /= u_norm;
    
    double t = a.x() * u.x() + a.y() * u.y();
    
    if(t < 0.0) return pt1;
    if(t > u_norm) return pt2;
    
    return t * u + pt1;
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

void RulerAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPointF mousePos;
    
    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    else {
        //...of course, you need to have access to a canvas-widget for that.//
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }
    
    // don't draw if invalid
    if (isAssistantComplete()) {
        QTransform initialTransform = converter->documentToWidgetTransform();

        // first we find the path that our point create.
        QPointF p1 = *handles()[0];
        QPointF p2 = *handles()[1];

        gc.setTransform(initialTransform);
        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2);
        //then we use this path to check the bounding rectangle//
        if (isSnappingActive() && path.boundingRect().contains(initialTransform.inverted().map(mousePos)) && previewVisible==true){
            drawPreview(gc, path);//and we draw the preview.
        }
    }
    gc.restore();
    
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}

void RulerAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false || !isAssistantComplete()){
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();

    // Draw the line
    QPointF p1 = *handles()[0];
    QPointF p2 = *handles()[1];

    gc.setTransform(initialTransform);
    QPainterPath path;
    path.moveTo(p1);
    path.lineTo(p2);
    drawPath(gc, path, isSnappingActive());
}

QPointF RulerAssistant::buttonPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool RulerAssistant::isAssistantComplete() const
{
    return handles().size() >= 2;
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

KisPaintingAssistant* RulerAssistantFactory::createPaintingAssistant() const
{
    return new RulerAssistant;
}
