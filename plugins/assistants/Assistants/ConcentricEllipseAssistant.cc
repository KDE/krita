/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#include <klocalizedstring.h>
#include "kis_debug.h"
#include <QPainter>
#include <QLinearGradient>
#include <QTransform>
#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>

ConcentricEllipseAssistant::ConcentricEllipseAssistant()
        : KisPaintingAssistant("concentric ellipse", i18n("Concentric Ellipse assistant"))
{
}

KisPaintingAssistantSP ConcentricEllipseAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new ConcentricEllipseAssistant(*this, handleMap));
}

ConcentricEllipseAssistant::ConcentricEllipseAssistant(const ConcentricEllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_ellipse(rhs.m_ellipse)
    , m_extraEllipse(rhs.m_extraEllipse)
{
}

QPointF ConcentricEllipseAssistant::project(const QPointF& pt, const QPointF& strokeBegin) const
{
    Q_ASSERT(isAssistantComplete());
    m_ellipse.set(*handles()[0], *handles()[1], *handles()[2]);

    qreal
            dx = pt.x() - strokeBegin.x(),
            dy = pt.y() - strokeBegin.y();
        if (dx * dx + dy * dy < 4.0) {
            // allow some movement before snapping
            return strokeBegin;
        }
    //calculate ratio
    QPointF initial = m_ellipse.project(strokeBegin);
    QPointF center = m_ellipse.boundingRect().center();
    qreal Ratio = QLineF(center, strokeBegin).length() /QLineF(center, initial).length();

    //calculate the points of the extrapolated ellipse.
    QLineF extrapolate0 = QLineF(center, *handles()[0]);
    extrapolate0.setLength(extrapolate0.length()*Ratio);
    QLineF extrapolate1 = QLineF(center, *handles()[1]);
    extrapolate1.setLength(extrapolate1.length()*Ratio);
    QLineF extrapolate2 = QLineF(center, *handles()[2]);
    extrapolate2.setLength(extrapolate2.length()*Ratio);

    //set the extrapolation ellipse.
    m_extraEllipse.set(extrapolate0.p2(), extrapolate1.p2(), extrapolate2.p2());

    return m_extraEllipse.project(pt);
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

    if (isSnappingActive() && previewVisible == true){

        if (isAssistantComplete()){
            if (m_ellipse.set(*handles()[0], *handles()[1], *handles()[2])) {
                QPointF initial = m_ellipse.project(initialTransform.inverted().map(mousePos));
                QPointF center = m_ellipse.boundingRect().center();
                qreal Ratio = QLineF(center, initialTransform.inverted().map(mousePos)).length() /QLineF(center, initial).length();
                //line from center to handle 1 * difference.
                //set handle1 translated to
                // valid ellipse
                gc.setTransform(initialTransform);
                gc.setTransform(m_ellipse.getInverse(), true);
                QPainterPath path;
                // Draw the ellipse
                path.addEllipse(QPointF(0, 0), m_ellipse.semiMajor()*Ratio, m_ellipse.semiMinor()*Ratio);
                drawPreview(gc, path);
            }
        }
    }
    gc.restore();
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}


void ConcentricEllipseAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false || handles().size() < 2) { // 2 points means a line, so we can continue after 1 point
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();

    if (handles().size() == 2) {
        // just draw the axis
        gc.setTransform(initialTransform);
        QPainterPath path;
        path.moveTo(*handles()[0]);
        path.lineTo(*handles()[1]);
        drawPath(gc, path, isSnappingActive());
        return;
    }

    if (m_ellipse.set(*handles()[0], *handles()[1], *handles()[2])) {
        // valid ellipse

        gc.setTransform(initialTransform);
        gc.setTransform(m_ellipse.getInverse(), true);
        QPainterPath path;
        path.moveTo(QPointF(-m_ellipse.semiMajor(), 0)); path.lineTo(QPointF(m_ellipse.semiMajor(), 0));
        path.moveTo(QPointF(0, -m_ellipse.semiMinor())); path.lineTo(QPointF(0, m_ellipse.semiMinor()));
        // Draw the ellipse
        path.addEllipse(QPointF(0, 0), m_ellipse.semiMajor(), m_ellipse.semiMinor());
        drawPath(gc, path, isSnappingActive());
    }
}

QRect ConcentricEllipseAssistant::boundingRect() const
{
    if (!isAssistantComplete()) {
        return KisPaintingAssistant::boundingRect();
    }

    if (m_ellipse.set(*handles()[0], *handles()[1], *handles()[2])) {
        return m_ellipse.boundingRect().adjusted(-2, -2, 2, 2).toAlignedRect();
    } else {
        return QRect();
    }
}

QPointF ConcentricEllipseAssistant::getEditorPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool ConcentricEllipseAssistant::isAssistantComplete() const
{
    return handles().size() >= 3;
}

void ConcentricEllipseAssistant::transform(const QTransform &transform)
{
    m_ellipse.set(*handles()[0], *handles()[1], *handles()[2]);

    QPointF newAxes;
    QTransform newTransform;

    std::tie(newAxes, newTransform) = KisAlgebra2D::transformEllipse(QPointF(m_ellipse.semiMajor(), m_ellipse.semiMinor()), m_ellipse.getInverse() * transform);

    const QPointF p1 = newTransform.map(QPointF(newAxes.x(), 0));
    const QPointF p2 = newTransform.map(QPointF(-newAxes.x(), 0));
    const QPointF p3 = newTransform.map(QPointF(0, newAxes.y()));

    *handles()[0] = p1;
    *handles()[1] = p2;
    *handles()[2] = p3;

    uncache();
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
