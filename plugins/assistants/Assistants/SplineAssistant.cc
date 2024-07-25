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
#include "KisBezierUtils.h"

#include <math.h>
#include <limits>
#include <algorithm>

struct GoldenSearchParams
{
    GoldenSearchParams(qreal lbound
                ,qreal ubound)
        :lbound(lbound)
        ,ubound(ubound)
    {
        samples = QVector<GoldenSearchPoint>(4);
    }

    struct GoldenSearchPoint {
        GoldenSearchPoint(qreal xval)
            : x(xval)
        {
        }

        GoldenSearchPoint(){}

        void inv_norm(qreal l, qreal u)
        {
            this->xnorm = this->x * (u - l) + l;
        }

        qreal fval;
        qreal xnorm;
        qreal x;
    };


    qreal lbound;
    qreal ubound;
    QVector<GoldenSearchPoint> samples;
};


struct SplineAssistant::Private {
    Private();

    QPointF prevStrokebegin;
    qreal prev_t {0};
};

SplineAssistant::Private::Private()
    : prevStrokebegin(0,0)
{
}

SplineAssistant::SplineAssistant()
    : KisPaintingAssistant("spline", i18n("Spline assistant"))
    , m_d(new Private)
{
}

SplineAssistant::SplineAssistant(const SplineAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_canvas(rhs.m_canvas)
    , m_d(new Private)
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


inline qreal goldenSearch(const QPointF& pt
                          , const QList<KisPaintingAssistantHandleSP> handles
                          , qreal low
                          , qreal high
                          , qreal tolerance
                          , uint max_iter)
{
    GoldenSearchParams ovalues = GoldenSearchParams(low,high);
    QVector<GoldenSearchParams::GoldenSearchPoint> p = ovalues.samples;
    qreal u = ovalues.ubound;
    qreal l = ovalues.lbound;

    const qreal ratio = 1 - 2/(1 + sqrt(5));
    p[0].x = 0;
    p[1].x = ratio;
    p[2].x = 1 - p[1].x;
    p[3].x = 1;

    p[1].inv_norm(l,u);
    p[2].inv_norm(l,u);

    p[1].fval = D(p[1].xnorm, *handles[0], *handles[2], *handles[3], *handles[1], pt);
    p[2].fval = D(p[2].xnorm, *handles[0], *handles[2], *handles[3], *handles[1], pt);

    GoldenSearchParams::GoldenSearchPoint xtemp = GoldenSearchParams::GoldenSearchPoint(0);

    uint i = 0; // used to force early exit
    while ( qAbs(p[2].xnorm - p[1].xnorm) > tolerance && i < max_iter) {

        if (p[1].fval < p[2].fval) {
            xtemp = p[1];
            p[3] = p[2];
            p[1].x = p[0].x + (p[2].x - p[1].x);
            p[2] = xtemp;

            p[1].inv_norm(l,u);
            p[1].fval = D(p[1].xnorm, *handles[0], *handles[2], *handles[3], *handles[1], pt);

        } else {
            xtemp = p[2];
            p[0] = p[1];
            p[2].x = p[1].x + (p[3].x - p[2].x);
            p[1] = xtemp;

            p[2].inv_norm(l,u);
            p[2].fval = D(p[2].xnorm, *handles[0], *handles[2], *handles[3], *handles[1], pt);

        }
        i++;
    }
    return (p[2].xnorm + p[1].xnorm) / 2;
}


QPointF SplineAssistant::project(const QPointF& pt, const QPointF& strokeBegin) const
{
    Q_ASSERT(isAssistantComplete());

    // minimize d(t), but keep t in the same neighbourhood as before (unless starting a new stroke)
    bool stayClose = (m_d->prevStrokebegin == strokeBegin)? true : false;
    qreal min_t;

    QList<QPointF> refs;
    QVector<int> hindex = {0,2,3,1}; // order handles as expected by KisBezierUtils
    Q_FOREACH(int i, hindex) {
        refs.append(*handles()[i]);
    }

    if (stayClose){
        // Search in the vicinity of previous t value.
        // This ensure unimodality for proper goldenSearch algorithm
        qreal delta = 1/10.0;
        qreal lbound = qBound(0.0,1.0, m_d->prev_t - delta);
        qreal ubound = qBound(0.0,1.0, m_d->prev_t + delta);
        min_t = goldenSearch(pt,handles(), lbound , ubound, 1e-6,1e+2);

    } else {
        min_t = KisBezierUtils::nearestPoint(refs,pt);
    }

    QPointF draw_pos = B(min_t, *handles()[0], *handles()[2], *handles()[3], *handles()[1]);

    m_d->prev_t = min_t;
    m_d->prevStrokebegin = strokeBegin;

    return draw_pos;
}

QPointF SplineAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/, qreal /*moveThresholdPt*/)
{
    return project(pt, strokeBegin);
}

void SplineAssistant::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    point = QPointF();
    strokeBegin = QPointF();
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
        bezierlinePen.setWidth(2);

        if (m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {

            if (!isSnappingActive()) {
                QColor snappingColor = assistantColor;
                snappingColor.setAlpha(snappingColor.alpha() * 0.2);

                bezierlinePen.setColor(snappingColor);
            }
            bezierlinePen.setCosmetic(true);

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

QPointF SplineAssistant::getDefaultEditorPosition() const
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
    return i18nc("A type of drawing assistants", "Spline");
}

KisPaintingAssistant* SplineAssistantFactory::createPaintingAssistant() const
{
    return new SplineAssistant;
}
