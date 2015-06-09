/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#include "ConcentricEllipseAssistant.h"

#include <klocale.h>
#include "kis_debug.h"
#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include "kis_coordinates_converter.h"

#include <math.h>

ConcentricEllipseAssistant::ConcentricEllipseAssistant()
        : KisPaintingAssistant("concentric ellipse", i18n("Concentric Ellipse assistant"))
{
}

QPointF ConcentricEllipseAssistant::project(const QPointF& pt, const QPointF& strokeBegin) const
{
    Q_ASSERT(handles().size() == 3);
    e.set(*handles()[0], *handles()[1], *handles()[2]);

    qreal
            dx = pt.x() - strokeBegin.x(),
            dy = pt.y() - strokeBegin.y();
        if (dx * dx + dy * dy < 4.0) {
            // allow some movement before snapping
            return strokeBegin;
        }
    //calculate ratio
    QPointF initial = e.project(strokeBegin);
    QPointF center = e.boundingRect().center();
    qreal Ratio = QLineF(center, strokeBegin).length() /QLineF(center, initial).length();

    //calculate the points of the extrapolated ellipse.
    QLineF extrapolate0 = QLineF(center, *handles()[0]);
    extrapolate0.setLength(extrapolate0.length()*Ratio);
    QLineF extrapolate1 = QLineF(center, *handles()[1]);
    extrapolate1.setLength(extrapolate1.length()*Ratio);
    QLineF extrapolate2 = QLineF(center, *handles()[2]);
    extrapolate2.setLength(extrapolate2.length()*Ratio);

    //set the extrapolation ellipse.
    extraE.set(extrapolate0.p2(), extrapolate1.p2(), extrapolate2.p2());

    return extraE.project(pt);
}

QPointF ConcentricEllipseAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);

}

void ConcentricEllipseAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
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
        dbgFile<<"canvas does not exist in the ellipse assistant, you may have passed arguments incorrectly:"<<canvas;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();

    if (outline()==true && previewVisible==true){
        if (handles().size() > 2){    
            if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
                QPointF initial = e.project(initialTransform.inverted().map(mousePos));
                QPointF center = e.boundingRect().center();
                qreal Ratio = QLineF(center, initialTransform.inverted().map(mousePos)).length() /QLineF(center, initial).length();
                //line from center to handle 1 * difference.
                //set handle1 translated to
                // valid ellipse
                gc.setTransform(initialTransform);
                gc.setTransform(e.getInverse(), true);
                QPainterPath path;
                // Draw the ellipse
                path.addEllipse(QPointF(0, 0), e.semiMajor()*Ratio, e.semiMinor()*Ratio);
                drawPreview(gc, path);
            }
        }
    }
    gc.restore();
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}


void ConcentricEllipseAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{

    if (assistantVisible==false){return;}
    if (handles().size() < 2) return;
        QTransform initialTransform = converter->documentToWidgetTransform();
    if (handles().size() == 2) {
        // just draw the axis
        gc.setTransform(initialTransform);
        QPainterPath path;
        path.moveTo(*handles()[0]);
        path.lineTo(*handles()[1]);
        drawPath(gc, path, snapping());
        return;
    }
    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        // valid ellipse

        gc.setTransform(initialTransform);
        gc.setTransform(e.getInverse(), true);
        QPainterPath path;
        path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
        path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
        // Draw the ellipse
        path.addEllipse(QPointF(0, 0), e.semiMajor(), e.semiMinor());
        drawPath(gc, path, snapping());
    }
}

QRect ConcentricEllipseAssistant::boundingRect() const
{
    if (handles().size() != 3) return KisPaintingAssistant::boundingRect();
    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        return e.boundingRect().adjusted(-2, -2, 2, 2).toAlignedRect();
    } else {
        return QRect();
    }
}

QPointF ConcentricEllipseAssistant::buttonPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

ConcentricEllipseAssistantFactory::ConcentricEllipseAssistantFactory()
{
}

ConcentricEllipseAssistantFactory::~ConcentricEllipseAssistantFactory()
{
}

QString ConcentricEllipseAssistantFactory::id() const
{
    return "concentric ellipse";
}

QString ConcentricEllipseAssistantFactory::name() const
{
    return i18n("Concentric Ellipse");
}

KisPaintingAssistant* ConcentricEllipseAssistantFactory::createPaintingAssistant() const
{
    return new ConcentricEllipseAssistant;
}
