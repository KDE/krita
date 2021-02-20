/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "EllipseAssistant.h"

#include <klocalizedstring.h>
#include "kis_debug.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_algebra_2d.h"

#include <math.h>

EllipseAssistant::EllipseAssistant()
        : KisPaintingAssistant("ellipse", i18n("Ellipse assistant"))
{
}

EllipseAssistant::EllipseAssistant(const EllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , e(rhs.e)
{
}

KisPaintingAssistantSP EllipseAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new EllipseAssistant(*this, handleMap));
}

QPointF EllipseAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(isAssistantComplete());
    e.set(*handles()[0], *handles()[1], *handles()[2]);
    return e.project(pt);
}

QPointF EllipseAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/)
{
    return project(pt);

}

void EllipseAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPoint mousePos;
    
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

    if (isSnappingActive() && boundingRect().contains(initialTransform.inverted().map(mousePos), false) && previewVisible==true){

        if (isAssistantComplete()){
            if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
                // valid ellipse
                gc.setTransform(initialTransform);
                gc.setTransform(e.getInverse(), true);
                QPainterPath path;
                //path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
                //path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
                // Draw the ellipse
                path.addEllipse(QPointF(0, 0), e.semiMajor(), e.semiMinor());
                drawPreview(gc, path);
            }
        }
    }
    gc.restore();
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}


void EllipseAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{

    if (assistantVisible == false || handles().size() < 2){
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
    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        // valid ellipse

        gc.setTransform(initialTransform);
        gc.setTransform(e.getInverse(), true);
        QPainterPath path;
        path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
        path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
        // Draw the ellipse
        path.addEllipse(QPointF(0, 0), e.semiMajor(), e.semiMinor());
        drawPath(gc, path, isSnappingActive());
    }
}

QRect EllipseAssistant::boundingRect() const
{
    if (!isAssistantComplete()) {
        return KisPaintingAssistant::boundingRect();
    }

    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        return e.boundingRect().adjusted(-2, -2, 2, 2).toAlignedRect();
    } else {
        return QRect();
    }
}

QPointF EllipseAssistant::getEditorPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool EllipseAssistant::isAssistantComplete() const
{
    return handles().size() >= 3;
}

void EllipseAssistant::transform(const QTransform &transform)
{
    e.set(*handles()[0], *handles()[1], *handles()[2]);

    QPointF newAxes;
    QTransform newTransform;

    std::tie(newAxes, newTransform) = KisAlgebra2D::transformEllipse(QPointF(e.semiMajor(), e.semiMinor()), e.getInverse() * transform);

    const QPointF p1 = newTransform.map(QPointF(newAxes.x(), 0));
    const QPointF p2 = newTransform.map(QPointF(-newAxes.x(), 0));
    const QPointF p3 = newTransform.map(QPointF(0, newAxes.y()));

    *handles()[0] = p1;
    *handles()[1] = p2;
    *handles()[2] = p3;

    uncache();
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

KisPaintingAssistant* EllipseAssistantFactory::createPaintingAssistant() const
{
    return new EllipseAssistant;
}
