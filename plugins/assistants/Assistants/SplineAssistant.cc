/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SplineAssistant.h"

#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_debug.h"

#include <math.h>
#include <limits>
#include <algorithm>

SplineAssistant::SplineAssistant()
    : KisPaintingAssistant("spline", i18n("Spline assistant"))
{
}

SplineAssistant::SplineAssistant(const SplineAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_canvas(rhs.m_canvas)
{
}

KisPaintingAssistantSP SplineAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new SplineAssistant(*this, handleMap));
}

// parametric form of a cubic spline (B(t) = (1-t)^3 P0 + 3 (1-t)^2 t P1 + 3 (1-t) t^2 P2 + t^3 P3)
inline QPointF B(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3)
{
    const qreal tp = 1 - t;
    const qreal tp2 = tp * tp;
    const qreal t2 = t * t;

    return  (    tp2 * tp) * P0 +
            (3 * tp2 * t ) * P1 +
            (3 * tp  * t2) * P2 +
            (    t   * t2) * P3;
}
// squared distance from a point on the spline to given point: we want to minimize this
inline qreal D(qreal t, const QPointF& P0, const QPointF& P1, const QPointF& P2, const QPointF& P3, const QPointF& p)
{
    const qreal
            tp =  1 - t,
            tp2 = tp * tp,
            t2 =  t * t,
            a =   tp2 * tp,
            b =   3 * tp2 * t,
            c =   3 * tp  * t2,
            d =   t   * t2,
            x_dist = a*P0.x() + b*P1.x() + c*P2.x() + d*P3.x() - p.x(),
            y_dist = a*P0.y() + b*P1.y() + c*P2.y() + d*P3.y() - p.y();

    return x_dist * x_dist + y_dist * y_dist;
}

QPointF SplineAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(isAssistantComplete());
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

void SplineAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPoint mousePos;
    
    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
        m_canvas = canvas;
    }
    else {
        //...of course, you need to have access to a canvas-widget for that.//
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in spline, you may have passed arguments incorrectly:"<<canvas;
    }
    

    if (handles().size() > 1) {

        QTransform initialTransform = converter->documentToWidgetTransform();

        // first we find the path that our point create.
        QPointF pts[4];
        pts[0] = *handles()[0];
        pts[1] = *handles()[1];
        pts[2] = (handles().size() >= 3) ? (*handles()[2]) : (*handles()[0]);
        pts[3] = (handles().size() >= 4) ? (*handles()[3]) : (handles().size() >= 3) ? (*handles()[2]) : (*handles()[1]);
        gc.setTransform(initialTransform);

        // Draw the spline
        QPainterPath path;
        path.moveTo(pts[0]);
        path.cubicTo(pts[2], pts[3], pts[1]);
        
        //then we use this path to check the bounding rectangle//
        if (isSnappingActive() && path.boundingRect().contains(initialTransform.inverted().map(mousePos)) && previewVisible==true){
            drawPreview(gc, path);//and we draw the preview.
        }
    }
    gc.restore();

   // there is some odd rectangle that is getting rendered when there is only one point, so don't start rendering the line until after 2
   // this issue only exists with this spline assistant...none of the others
   if (handles().size() > 2) {
      KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
   }
}

void SplineAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false || handles().size() < 2 ){
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();

    QPointF pts[4];
    pts[0] = *handles()[0];
    pts[1] = *handles()[1];
    pts[2] = (handles().size() >= 3) ? (*handles()[2]) : (*handles()[0]);
    pts[3] = (handles().size() >= 4) ? (*handles()[3]) : (handles().size() >= 3) ? (*handles()[2]) : (*handles()[1]);

    gc.setTransform(initialTransform);


    {  // Draw bezier handles control lines only if we are editing the assistant
        gc.save();
        QColor assistantColor = effectiveAssistantColor();
        QPen bezierlinePen(assistantColor);
        bezierlinePen.setStyle(Qt::DotLine);
        bezierlinePen.setWidth(1);

        if (m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {

            if (!isSnappingActive()) {
                QColor snappingColor = assistantColor;
                snappingColor.setAlpha(snappingColor.alpha() * 0.2);

                bezierlinePen.setColor(snappingColor);
            }

            gc.setPen(bezierlinePen);
            gc.drawLine(pts[0], pts[2]);

            if (isAssistantComplete()) {
                gc.drawLine(pts[1], pts[3]);
            }
            gc.setPen(QColor(0, 0, 0, 125));
        }
        gc.restore();
    }


    // Draw the spline
    QPainterPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[2], pts[3], pts[1]);
    drawPath(gc, path, isSnappingActive());


}

QPointF SplineAssistant::getEditorPosition() const
{
    return B(0.5, *handles()[0], *handles()[2], *handles()[3], *handles()[1]);
}

bool SplineAssistant::isAssistantComplete() const
{
    return handles().size() >= 4; // specify 4 corners to make assistant complete
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
