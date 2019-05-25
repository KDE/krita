/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#include "PerspectiveAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>
#include <limits>

PerspectiveAssistant::PerspectiveAssistant(QObject *parent)
    : KisAbstractPerspectiveGrid(parent)
    , KisPaintingAssistant("perspective", i18n("Perspective assistant"))
{
}

PerspectiveAssistant::PerspectiveAssistant(const PerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandlebSP> &handleMap)
    : KisAbstractPerspectiveGrid(rhs.parent())
    , KisPaintingAssistant(rhs, handleMap)
    , m_snapLine(rhs.m_snapLine)
    , m_cachedTransform(rhs.m_cachedTransform)
    , m_cachedPolygon(rhs.m_cachedPolygon)
    , m_cacheValid(rhs.m_cacheValid)
{
    for (int i = 0; i < 4; ++i) {
        m_cachedPoints[i] = rhs.m_cachedPoints[i];
    }
}

KisPaintingAssistantSP PerspectiveAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new PerspectiveAssistant(*this, handleMap));
}
// squared distance from a point to a line
inline qreal distsqr(const QPointF& pt, const QLineF& line)
{
    // distance = |(p2 - p1) x (p1 - pt)| / |p2 - p1|

    // magnitude of (p2 - p1) x (p1 - pt)
    const qreal cross = (line.dx() * (line.y1() - pt.y()) - line.dy() * (line.x1() - pt.x()));

    return cross * cross / (line.dx() * line.dx() + line.dy() * line.dy());
}

QPointF PerspectiveAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    const static QPointF nullPoint(std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());

    Q_ASSERT(isAssistantComplete());

    if (m_snapLine.isNull()) {
        QPolygonF poly;
        QTransform transform;

        if (!getTransform(poly, transform)) {
            return nullPoint;
        }

        if (!poly.containsPoint(strokeBegin, Qt::OddEvenFill)) {
            return nullPoint; // avoid problems with multiple assistants: only snap if starting in the grid
        }


        const qreal dx = pt.x() - strokeBegin.x();
        const qreal dy = pt.y() - strokeBegin.y();

        if (dx * dx + dy * dy < 4.0) {
            return strokeBegin; // allow some movement before snapping
        }

        // construct transformation
        bool invertible;
        const QTransform inverse = transform.inverted(&invertible);
        if (!invertible) {
            return nullPoint; // shouldn't happen
        }


        // figure out which direction to go
        const QPointF start = inverse.map(strokeBegin);
        const QLineF verticalLine = QLineF(strokeBegin, transform.map(start + QPointF(0, 1)));
        const QLineF horizontalLine = QLineF(strokeBegin, transform.map(start + QPointF(1, 0)));

        // determine whether the horizontal or vertical line is closer to the point
        m_snapLine = distsqr(pt, verticalLine) < distsqr(pt, horizontalLine) ? verticalLine : horizontalLine;
    }

    // snap to line
    const qreal
            dx = m_snapLine.dx(),
            dy = m_snapLine.dy(),
            dx2 = dx * dx,
            dy2 = dy * dy,
            invsqrlen = 1.0 / (dx2 + dy2);
    QPointF r(dx2 * pt.x() + dy2 * m_snapLine.x1() + dx * dy * (pt.y() - m_snapLine.y1()),
              dx2 * m_snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - m_snapLine.x1()));

    r *= invsqrlen;
    return r;
}

QPointF PerspectiveAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);
}

void PerspectiveAssistant::endStroke()
{
    m_snapLine = QLineF();
}

bool PerspectiveAssistant::contains(const QPointF& pt) const
{
    QPolygonF poly;
    if (!quad(poly)) return false;
    return poly.containsPoint(pt, Qt::OddEvenFill);
}

inline qreal lengthSquared(const QPointF& vector)
{
    return vector.x() * vector.x() + vector.y() * vector.y();
}

inline qreal localScale(const QTransform& transform, QPointF pt)
{
    //    const qreal epsilon = 1e-5, epsilonSquared = epsilon * epsilon;
    //    qreal xSizeSquared = lengthSquared(transform.map(pt + QPointF(epsilon, 0.0)) - orig) / epsilonSquared;
    //    qreal ySizeSquared = lengthSquared(transform.map(pt + QPointF(0.0, epsilon)) - orig) / epsilonSquared;
    //    xSizeSquared /= lengthSquared(transform.map(QPointF(0.0, pt.y())) - transform.map(QPointF(1.0, pt.y())));
    //    ySizeSquared /= lengthSquared(transform.map(QPointF(pt.x(), 0.0)) - transform.map(QPointF(pt.x(), 1.0)));
    //  when taking the limit epsilon->0:
    //  xSizeSquared=((m23*y+m33)^2*(m23*y+m33+m13)^2)/(m23*y+m13*x+m33)^4
    //  ySizeSquared=((m23*y+m33)^2*(m23*y+m33+m13)^2)/(m23*y+m13*x+m33)^4
    //  xSize*ySize=(abs(m13*x+m33)*abs(m13*x+m33+m23)*abs(m23*y+m33)*abs(m23*y+m33+m13))/(m23*y+m13*x+m33)^4
    const qreal x = transform.m13() * pt.x(),
            y = transform.m23() * pt.y(),
            a = x + transform.m33(),
            b = y + transform.m33(),
            c = x + y + transform.m33(),
            d = c * c;
    return fabs(a*(a + transform.m23())*b*(b + transform.m13()))/(d * d);
}

// returns the reciprocal of the maximum local scale at the points (0,0),(0,1),(1,0),(1,1)
inline qreal inverseMaxLocalScale(const QTransform& transform)
{
    const qreal a = fabs((transform.m33() + transform.m13()) * (transform.m33() + transform.m23())),
            b = fabs((transform.m33()) * (transform.m13() + transform.m33() + transform.m23())),
            d00 = transform.m33() * transform.m33(),
            d11 = (transform.m33() + transform.m23() + transform.m13())*(transform.m33() + transform.m23() + transform.m13()),
            s0011 = qMin(d00, d11) / a,
            d10 = (transform.m33() + transform.m13()) * (transform.m33() + transform.m13()),
            d01 = (transform.m33() + transform.m23()) * (transform.m33() + transform.m23()),
            s1001 = qMin(d10, d01) / b;
    return qMin(s0011, s1001);
}

qreal PerspectiveAssistant::distance(const QPointF& pt) const
{
    QPolygonF poly;
    QTransform transform;

    if (!getTransform(poly, transform)) {
        return 1.0;
    }

    bool invertible;
    QTransform inverse = transform.inverted(&invertible);

    if (!invertible) {
        return 1.0;
    }

    if (inverse.m13() * pt.x() + inverse.m23() * pt.y() + inverse.m33() == 0.0) {
        return 0.0; // point at infinity
    }

    return localScale(transform, inverse.map(pt)) * inverseMaxLocalScale(transform);
}

// draw a vanishing point marker
inline QPainterPath drawX(const QPointF& pt)
{
    QPainterPath path;
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() - 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() + 5.0));
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() + 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() - 5.0));
    return path;
}

void PerspectiveAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QTransform initialTransform = converter->documentToWidgetTransform();
    //QTransform reverseTransform = converter->widgetToDocument();
    QPolygonF poly;
    QTransform transform; // unused, but computed for caching purposes
    if (getTransform(poly, transform) && assistantVisible==true) {
        // draw vanishing points
        QPointF intersection(0, 0);
        if (fmod(QLineF(poly[0], poly[1]).angle(), 180.0)>=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)+2.0 || fmod(QLineF(poly[0], poly[1]).angle(), 180.0)<=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)-2.0) {
            if (QLineF(poly[0], poly[1]).intersect(QLineF(poly[2], poly[3]), &intersection) != QLineF::NoIntersection) {
                drawPath(gc, drawX(initialTransform.map(intersection)));
            }
        }
        if (fmod(QLineF(poly[1], poly[2]).angle(), 180.0)>=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)+2.0 || fmod(QLineF(poly[1], poly[2]).angle(), 180.0)<=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)-2.0){
            if (QLineF(poly[1], poly[2]).intersect(QLineF(poly[3], poly[0]), &intersection) != QLineF::NoIntersection) {
                drawPath(gc, drawX(initialTransform.map(intersection)));
            }
        }
    }

    if (isSnappingActive() && getTransform(poly, transform) && previewVisible==true){
        //find vanishing point, find mouse, draw line between both.
        QPainterPath path2;
        QPointF intersection(0, 0);//this is the position of the vanishing point.
        QPointF mousePos(0,0);
        QLineF snapLine;
        QRect viewport= gc.viewport();
        QRect bounds;
        
        if (canvas){
            //simplest, cheapest way to get the mouse-position
            mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
        }
        else {
            //...of course, you need to have access to a canvas-widget for that.
            mousePos = QCursor::pos(); // this'll give an offset
            dbgFile<<"canvas does not exist, you may have passed arguments incorrectly:"<<canvas;
        }
        //figure out if point is in the perspective grid
        QPointF intersectTransformed(0, 0); // dummy for holding transformed intersection so the code is more readable.

        if (poly.containsPoint(initialTransform.inverted().map(mousePos), Qt::OddEvenFill)==true){
            // check if the lines aren't parallel to each other to avoid calculation errors in the intersection calculation (bug 345754)//
            if (fmod(QLineF(poly[0], poly[1]).angle(), 180.0)>=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)+2.0 || fmod(QLineF(poly[0], poly[1]).angle(), 180.0)<=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)-2.0) {
                if (QLineF(poly[0], poly[1]).intersect(QLineF(poly[2], poly[3]), &intersection) != QLineF::NoIntersection) {
                    intersectTransformed = initialTransform.map(intersection);
                    snapLine = QLineF(intersectTransformed, mousePos);
                    KisAlgebra2D::intersectLineRect(snapLine, viewport);
                    bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());
                    QPainterPath path;

                    if (bounds.contains(intersectTransformed.toPoint())){
                        path2.moveTo(intersectTransformed);
                        path2.lineTo(snapLine.p1());
                    }
                    else {
                        path2.moveTo(snapLine.p1());
                        path2.lineTo(snapLine.p2());
                    }
                }
            }
            if (fmod(QLineF(poly[1], poly[2]).angle(), 180.0)>=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)+2.0 || fmod(QLineF(poly[1], poly[2]).angle(), 180.0)<=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)-2.0){
                if (QLineF(poly[1], poly[2]).intersect(QLineF(poly[3], poly[0]), &intersection) != QLineF::NoIntersection) {
                    intersectTransformed = initialTransform.map(intersection);
                    snapLine = QLineF(intersectTransformed, mousePos);
                    KisAlgebra2D::intersectLineRect(snapLine, viewport);
                    bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());
                    QPainterPath path;

                    if (bounds.contains(intersectTransformed.toPoint())){
                        path2.moveTo(intersectTransformed);
                        path2.lineTo(snapLine.p1());
                    }
                    else {
                        path2.moveTo(snapLine.p1());
                        path2.lineTo(snapLine.p2());
                    }
                }
            }
            drawPreview(gc, path2);
        }
    }

    gc.restore();

    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached,canvas, assistantVisible, previewVisible);
}

void PerspectiveAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false) {
        return;
    }

    gc.setTransform(converter->documentToWidgetTransform());
    QPolygonF poly;
    QTransform transform;

    if (!getTransform(poly, transform)) {
        // color red for an invalid transform, but not for an incomplete one
        if(isAssistantComplete()) {
            gc.setPen(QColor(255, 0, 0, 125));
            gc.drawPolygon(poly);
        } else {
            QPainterPath path;
            path.addPolygon(poly);
            drawPath(gc, path, isSnappingActive());
        }
    } else {
        gc.setPen(QColor(0, 0, 0, 125));
        gc.setTransform(transform, true);
        QPainterPath path;
        for (int y = 0; y <= 8; ++y)
        {
            path.moveTo(QPointF(0.0, y * 0.125));
            path.lineTo(QPointF(1.0, y * 0.125));
        }
        for (int x = 0; x <= 8; ++x)
        {
            path.moveTo(QPointF(x * 0.125, 0.0));
            path.lineTo(QPointF(x * 0.125, 1.0));
        }
        drawPath(gc, path, isSnappingActive());
    }
    
}

QPointF PerspectiveAssistant::buttonPosition() const
{
    QPointF centroid(0, 0);
    for (int i = 0; i < 4; ++i) {
        centroid += *handles()[i];
    }

    return centroid * 0.25;
}

template <typename T> int sign(T a)
{
    return (a > 0) - (a < 0);
}
// perpendicular dot product
inline qreal pdot(const QPointF& a, const QPointF& b)
{
    return a.x() * b.y() - a.y() * b.x();
}

bool PerspectiveAssistant::quad(QPolygonF& poly) const
{
    for (int i = 0; i < handles().size(); ++i) {
        poly.push_back(*handles()[i]);
    }

    if (!isAssistantComplete()) {
        return false;
    }

    int sum = 0;
    int signs[4];

    for (int i = 0; i < 4; ++i) {
        int j = (i == 3) ? 0 : (i + 1);
        int k = (j == 3) ? 0 : (j + 1);
        signs[i] = sign(pdot(poly[j] - poly[i], poly[k] - poly[j]));
        sum += signs[i];
    }

    if (sum == 0) {
        // complex (crossed)
        for (int i = 0; i < 4; ++i) {
            int j = (i == 3) ? 0 : (i + 1);
            if (signs[i] * signs[j] == -1) {
                // opposite signs: uncross
                std::swap(poly[i], poly[j]);
                return true;
            }
        }
        // okay, maybe it's just a line
        return false;
    } else if (sum != 4 && sum != -4) {
        // concave, or a triangle
        if (sum == 2 || sum == -2) {
            // concave, let's return a triangle instead
            for (int i = 0; i < 4; ++i) {
                int j = (i == 3) ? 0 : (i + 1);
                if (signs[i] != sign(sum)) {
                    // wrong sign: drop the inside node
                    poly.remove(j);
                    return false;
                }
            }
        }
        return false;
    }
    // convex
    return true;
}

bool PerspectiveAssistant::getTransform(QPolygonF& poly, QTransform& transform) const
{
    if (m_cachedPolygon.size() != 0 && isAssistantComplete()) {
        for (int i = 0; i <= 4; ++i) {
            if (i == 4) {
                poly = m_cachedPolygon;
                transform = m_cachedTransform;
                return m_cacheValid;
            }
            if (m_cachedPoints[i] != *handles()[i]) break;
        }
    }

    m_cachedPolygon.clear();
    m_cacheValid = false;

    if (!quad(poly)) {
        m_cachedPolygon = poly;
        return false;
    }

    if (!QTransform::squareToQuad(poly, transform)) {
        qWarning("Failed to create perspective mapping");
        return false;
    }

    for (int i = 0; i < 4; ++i) {
        m_cachedPoints[i] = *handles()[i];
    }

    m_cachedPolygon = poly;
    m_cachedTransform = transform;
    m_cacheValid = true;
    return true;
}

bool PerspectiveAssistant::isAssistantComplete() const
{
    return handles().size() >= 4; // specify 4 corners to make assistant complete
}



PerspectiveAssistantFactory::PerspectiveAssistantFactory()
{
}

PerspectiveAssistantFactory::~PerspectiveAssistantFactory()
{
}

QString PerspectiveAssistantFactory::id() const
{
    return "perspective";
}

QString PerspectiveAssistantFactory::name() const
{
    return i18n("Perspective");
}

KisPaintingAssistant* PerspectiveAssistantFactory::createPaintingAssistant() const
{
    return new PerspectiveAssistant;
}
