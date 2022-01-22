/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 * SPDX-FileCopyrightText: 2022 Julian Schmidt <julisch1107@web.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "InfiniteRulerAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>

InfiniteRulerAssistant::InfiniteRulerAssistant()
    : RulerAssistant("infinite ruler", i18n("Infinite Ruler assistant"))
{
}

InfiniteRulerAssistant::InfiniteRulerAssistant(const InfiniteRulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : RulerAssistant(rhs, handleMap)
{
}

KisPaintingAssistantSP InfiniteRulerAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new InfiniteRulerAssistant(*this, handleMap));
}

QPointF InfiniteRulerAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    Q_ASSERT(isAssistantComplete());
    //code nicked from the perspective ruler.
    qreal
            dx = pt.x() - strokeBegin.x(),
            dy = pt.y() - strokeBegin.y();
        if (dx * dx + dy * dy < 4.0) {
            // allow some movement before snapping
            return strokeBegin;
        }
    //dbgKrita<<strokeBegin<< ", " <<*handles()[0];
    QLineF snapLine = QLineF(*handles()[0], *handles()[1]);
    
    
        dx = snapLine.dx();
        dy = snapLine.dy();
    const qreal
        dx2 = dx * dx,
        dy2 = dy * dy,
        invsqrlen = 1.0 / (dx2 + dy2);
    QPointF r(dx2 * pt.x() + dy2 * snapLine.x1() + dx * dy * (pt.y() - snapLine.y1()),
              dx2 * snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - snapLine.x1()));
    r *= invsqrlen;
    return r;
    //return pt;
}

QPointF InfiniteRulerAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/)
{
    return project(pt, strokeBegin);
}

void InfiniteRulerAssistant::drawSubdivisions(QPainter& gc, const KisCoordinatesConverter *converter) {
    if (subdivisions() == 0) {
        return;
    }
    
    // Get handle positions
    QTransform document2widget = converter->documentToWidgetTransform();
    
    QPointF p1 = document2widget.map(*handles()[0]);
    QPointF p2 = document2widget.map(*handles()[1]);
    
    const qreal scale = 16.0 / 2;
    const qreal minorScale = scale / 2;
    const QRectF clipping = QRectF(gc.viewport()).adjusted(-scale, -scale, scale, scale);
    // If the lines would end up closer to each other than this threshold (in
    // screen coordinates), they are not rendered, as they wouldn't be
    // distinguishable anymore.
    const qreal threshold = 3.0;
    
    // Calculate line direction and normal vector
    QPointF delta = p2 - p1;
    qreal length = sqrt(KisPaintingAssistant::norm2(delta));
    qreal stepsize = length / subdivisions();
    
    // Only draw if lines are far enough apart
    if (stepsize >= threshold) {
        QPointF normal = QPointF(delta.y(), -delta.x());
        normal /= length;
        
        // Clip the line to the viewport and find the t parameters for these
        // points
        ClippingResult res = clipLineParametric(QLineF(p1, p2), clipping);
        // Abort if line is outside clipping area
        if (!res.intersects) {
            return;
        }
        // Calculate indices to start and end the subdivisions on screen by
        // rounding further "away" from the visible area, ensuring that all
        // divisions that could be visible are actually drawn
        int istart = (int) floor(res.tmin * subdivisions());
        int iend =   (int)  ceil(res.tmax * subdivisions());
  
        QPainterPath path;
        QPainterPath highlight;
        
        // Draw the major subdivisions
        for (int ii = istart; ii < iend; ++ii) {
            QPointF pos = p1 + delta * ((qreal)ii / subdivisions());
            // No additional clipping check needed, since we're already
            // constrained inside it by the ii values.
            // However, don't draw over the thicker lines where the actual
            // ruler is located and already drawn!
            if (0 <= ii && ii < subdivisions()) {
                continue;
            }
            // Special case at ii == subdivs: minor subdivisions are needed
            // here, but not the major one, as this is the last line drawn of
            // the main ruler.
            if (ii != subdivisions()) {
                // Highlight the integer multiples of the ruler length
                if (ii % subdivisions() == 0) {
                    highlight.moveTo(pos - normal * scale);
                    highlight.lineTo(pos + normal * scale);
                } else {
                    path.moveTo(pos - normal * scale);
                    path.lineTo(pos + normal * scale);
                }
            }
  
            // Draw minor subdivisions, if they exist (implicit check due to
            // the loop bounds)
            // Skip if major subdivisions are too close already
            if (stepsize / minorSubdivisions() < threshold)
                continue;
            // Draw minor marks in between the major ones
            for (int jj = 1; jj < minorSubdivisions(); ++jj) {
                QPointF mpos = pos + delta * ((qreal)jj / (subdivisions() * minorSubdivisions()));
        
                path.moveTo(mpos - normal * minorScale);
                path.lineTo(mpos + normal * minorScale);
            }
        }
  
        // Draw highlight as regular path (2 px wide)
        drawPath(gc, highlight);
        // Draw normal lines as preview (1 px wide)
        drawPreview(gc, path);
    }
}

void InfiniteRulerAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    
    if (isAssistantComplete() && isSnappingActive() && previewVisible) {
      
        // Extend the line to the full viewport
        QTransform initialTransform = converter->documentToWidgetTransform();
        QLineF snapLine = QLineF(initialTransform.map(*handles()[0]), initialTransform.map(*handles()[1]));
        QRect viewport = gc.viewport();
        KisAlgebra2D::intersectLineRect(snapLine, viewport, true);
  
        // Draw as preview (thin lines)
        QPainterPath path;
        path.moveTo(snapLine.p1());
        path.lineTo(snapLine.p2());
        drawPreview(gc, path);
        
        // Add the extended subdivisions, if active
        // When the number of subdivisions (or minor subdivisions) is set to
        // 0, the respective feature is turned off and won't be rendered.
        if (subdivisions() > 0) {
            drawSubdivisions(gc, converter);
        }
    }
    
    gc.restore();
    
    RulerAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
}

InfiniteRulerAssistant::ClippingResult InfiniteRulerAssistant::clipLineParametric(QLineF line, QRectF rect, bool extendFirst, bool extendSecond) {
    double dx = line.x2() - line.x1();
    double dy = line.y2() - line.y1();
    
    double q1 = line.x1() - rect.x();
    double q2 = rect.x() + rect.width() - line.x1();
    double q3 = line.y1() - rect.y();
    double q4 = rect.y() + rect.height() - line.y1();
    
    QVector<double> p = QVector<double>({-dx, dx, -dy, dy});
    QVector<double> q = QVector<double>({q1, q2, q3, q4});
    
    double tmin = extendFirst ? -std::numeric_limits<double>::infinity() : 0.0;
    double tmax = extendSecond ? +std::numeric_limits<double>::infinity() : 1.0;
    
    for (int i = 0; i < p.length(); i++) {
        
        if (p[i] == 0 && q[i] < 0) {
            // Line is parallel to this boundary and outside of it
            return ClippingResult{false, 0, 0};
            
        } else if (p[i] < 0) {
            // Line moves into this boundary with increasing t
            // Set minimum t where it just comes in
            double t = q[i] / p[i];
            if (t > tmin) {
                tmin = t;
            }
          
        } else if (p[i] > 0) {
            // Line moves out of this boundary with increasing t
            // Set maximum t where it is still inside
            double t = q[i] / p[i];
            if (t < tmax) {
                tmax = t;
            }
        }
    }
    
    // The line intersects the rectangle if tmin < tmax.
    return ClippingResult{tmin < tmax, tmin, tmax};
}

QPointF InfiniteRulerAssistant::getEditorPosition() const
{
    return (*handles()[0]);
}

bool InfiniteRulerAssistant::isAssistantComplete() const
{
    return handles().size() >= 2;
}

InfiniteRulerAssistantFactory::InfiniteRulerAssistantFactory() = default;

InfiniteRulerAssistantFactory::~InfiniteRulerAssistantFactory() = default;

QString InfiniteRulerAssistantFactory::id() const
{
    return "infinite ruler";
}

QString InfiniteRulerAssistantFactory::name() const
{
    return i18n("Infinite Ruler");
}

KisPaintingAssistant* InfiniteRulerAssistantFactory::createPaintingAssistant() const
{
    return new InfiniteRulerAssistant;
}
