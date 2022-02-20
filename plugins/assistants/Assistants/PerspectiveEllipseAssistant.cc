/*
 * SPDX-FileCopyrightText: 2022 Srirupa Datta <srirupa.sps@gmail.com>
 */

#include "PerspectiveEllipseAssistant.h"

#include <klocalizedstring.h>
#include "kis_debug.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_algebra_2d.h"
#include <Eigen/Eigenvalues>

#include <math.h>
#include<QDebug>
#include <QtMath>

PerspectiveEllipseAssistant::PerspectiveEllipseAssistant()
    : KisPaintingAssistant("perspective ellipse", i18n("Perspective Ellipse assistant"))
    , m_followBrushPosition(false)
    , m_adjustedPositionValid(false)
{
}

PerspectiveEllipseAssistant::PerspectiveEllipseAssistant(const PerspectiveEllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_snapLine(rhs.m_snapLine)
    , e(rhs.e)
    , m_cachedTransform(rhs.m_cachedTransform)
    , m_cachedEllipseTransform(rhs.m_cachedEllipseTransform)
    , m_cachedPolygon(rhs.m_cachedPolygon)
    , m_cacheValid(rhs.m_cacheValid)
    , m_followBrushPosition(rhs.m_followBrushPosition)
    , m_adjustedPositionValid(rhs.m_adjustedPositionValid)
    , m_adjustedBrushPosition(rhs.m_adjustedBrushPosition)
{
    for (int i = 0; i < 4; ++i) {
        m_cachedPoints[i] = rhs.m_cachedPoints[i];
    }
    
    for (int i = 0; i < 3; ++i) {
        m_cachedEllipsePoints[i] = rhs.m_cachedEllipsePoints[i];
    }
}

KisPaintingAssistantSP PerspectiveEllipseAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new PerspectiveEllipseAssistant(*this, handleMap));
}

void PerspectiveEllipseAssistant::setAdjustedBrushPosition(const QPointF position)
{
    m_adjustedBrushPosition = position;
    m_adjustedPositionValid = true;
}

void PerspectiveEllipseAssistant::setFollowBrushPosition(bool follow)
{
    m_followBrushPosition = follow;
}

inline qreal distsqr(const QPointF& pt, const QLineF& line)
{
    // distance = |(p2 - p1) x (p1 - pt)| / |p2 - p1|

    // magnitude of (p2 - p1) x (p1 - pt)
    const qreal cross = (line.dx() * (line.y1() - pt.y()) - line.dy() * (line.x1() - pt.x()));

    return cross * cross / (line.dx() * line.dx() + line.dy() * line.dy());
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
// draw a vanishing point marker
inline QPainterPath drawX(const QPointF& pt)
{
    QPainterPath path;
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() - 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() + 5.0));
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() + 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() - 5.0));
    return path;
}

QPointF PerspectiveEllipseAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    const static QPointF nullPoint(std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());

    Q_ASSERT(isAssistantComplete());

    e.set(m_cachedEllipsePoints[0], m_cachedEllipsePoints[1], m_cachedEllipsePoints[2]);
    return e.project(pt);
}

QPointF PerspectiveEllipseAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/)
{
    return project(pt, strokeBegin);
}

void PerspectiveEllipseAssistant::endStroke()
{
    // Brush stroke ended, guides should follow the brush position again.
    m_followBrushPosition = false;
    m_adjustedPositionValid = false;

    m_snapLine = QLineF();
}

void PerspectiveEllipseAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPoint mousePos;
    
    QTransform initialTransform = converter->documentToWidgetTransform();

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
        // find vanishing point, find mouse, draw line between both.
        QPainterPath path2;
        QPointF intersection(0, 0);//this is the position of the vanishing point.
        QPointF mousePos(0,0);
        QLineF snapLine;
        QRect viewport= gc.viewport();
        QRect bounds;

        if (canvas){
            // simplest, cheapest way to get the mouse-position
            mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
        }
        else {
            // ...of course, you need to have access to a canvas-widget for that.
            mousePos = QCursor::pos(); // this'll give an offset
            dbgFile<<"canvas does not exist, you may have passed arguments incorrectly:"<<canvas;
        }
        
        if (m_followBrushPosition && m_adjustedPositionValid) {
            mousePos = initialTransform.map(m_adjustedBrushPosition);
        }

        //figure out if point is in the perspective mesh
        QPointF intersectTransformed(0, 0); // dummy for holding transformed intersection so the code is more readable.

        if (poly.containsPoint(initialTransform.inverted().map(mousePos), Qt::OddEvenFill)==true){
            // check if the lines aren't parallel to each other to avoid calculation errors in the intersection calculation (bug 345754)//
//             if (fmod(QLineF(poly[0], poly[1]).angle(), 180.0)>=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)+2.0 || fmod(QLineF(poly[0], poly[1]).angle(), 180.0)<=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)-2.0) {
//                 if (QLineF(poly[0], poly[1]).intersect(QLineF(poly[2], poly[3]), &intersection) != QLineF::NoIntersection) {
//                     intersectTransformed = initialTransform.map(intersection);
//                     snapLine = QLineF(intersectTransformed, mousePos);
//                     KisAlgebra2D::intersectLineRect(snapLine, viewport, true);
//                     bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());
// 
//                     if (bounds.contains(intersectTransformed.toPoint())){
//                         path2.moveTo(intersectTransformed);
//                         path2.lineTo(snapLine.p2());
//                     }
//                     else {
//                         path2.moveTo(snapLine.p1());
//                         path2.lineTo(snapLine.p2());
//                     }
//                 }
//             }
//             if (fmod(QLineF(poly[1], poly[2]).angle(), 180.0)>=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)+2.0 || fmod(QLineF(poly[1], poly[2]).angle(), 180.0)<=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)-2.0){
//                 if (QLineF(poly[1], poly[2]).intersect(QLineF(poly[3], poly[0]), &intersection) != QLineF::NoIntersection) {
//                     intersectTransformed = initialTransform.map(intersection);
//                     snapLine = QLineF(intersectTransformed, mousePos);
//                     KisAlgebra2D::intersectLineRect(snapLine, viewport, true);
//                     bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());
//                     QPainterPath path;
// 
//                     if (bounds.contains(intersectTransformed.toPoint())){
//                         path2.moveTo(intersectTransformed);
//                         path2.lineTo(snapLine.p1());
//                     }
//                     else {
//                         path2.moveTo(snapLine.p1());
//                         path2.lineTo(snapLine.p2());
//                     }
//                 }
//             }
//             drawPreview(gc, path2);
        }
    }
    
    if (isSnappingActive() && boundingRect().contains(initialTransform.inverted().map(mousePos), false) && previewVisible==true){

        if (isAssistantComplete()){
            if (e.set(m_cachedEllipsePoints[0], m_cachedEllipsePoints[1], m_cachedEllipsePoints[2])) {
                // valid ellipse
                gc.setTransform(initialTransform * m_cachedEllipseTransform);
//                 gc.setTransform(e.getInverse(), true);
                
                QPainterPath path;
                path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
                path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
                // Draw the ellipse
                path.addEllipse(QPointF(0.0, 0.0), e.semiMajor(), e.semiMinor());
                drawPreview(gc, path);
            }
        }
    } 

    // draw the lines themselves
    gc.setTransform(converter->documentToWidgetTransform());

    if (assistantVisible) {
        // getTransform was checked before but what if the preview wasn't visible etc., and we need a return value here too
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
            for (int y = 0; y <= 1; ++y)
            {
                QLineF line = QLineF(QPointF(0.0, y), QPointF(1.0, y));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }
            for (int x = 0; x <= 1; ++x)
            {
                QLineF line = QLineF(QPointF(x, 0.0), QPointF(x, 1.0));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }
            
            path.addEllipse(QPointF(0.5, 0.5), 0.5, 0.5);
            drawPath(gc, path, isSnappingActive());
        }
    }
    
    gc.restore();
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}


void PerspectiveEllipseAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    QTransform initialTransform = converter->documentToWidgetTransform();
    
    if (e.set(m_cachedEllipsePoints[0], m_cachedEllipsePoints[1], m_cachedEllipsePoints[2])) {
        // valid ellipse
        gc.setTransform(initialTransform * m_cachedEllipseTransform);
//         gc.setTransform(e.getInverse(), true);

        QPainterPath path;
        path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
        path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
        // Draw the ellipse
        path.addEllipse(QPointF(0.0, 0.0), e.semiMajor(), e.semiMinor());
        drawPath(gc, path, isSnappingActive());
    }
    
}

QRect PerspectiveEllipseAssistant::boundingRect() const
{
    // INCOMPLETE
     if (!isAssistantComplete()) {
       return KisPaintingAssistant::boundingRect();
    }

    if (e.set(m_cachedEllipsePoints[0], m_cachedEllipsePoints[1], m_cachedEllipsePoints[2])) {
       return e.boundingRect().adjusted(-2, -2, 2, 2).toAlignedRect();
    } else {
       return QRect();
    }
}

QPointF PerspectiveEllipseAssistant::getEditorPosition() const
{
     QPointF centroid(0, 0);
    for (int i = 0; i < 4; ++i) {
        centroid += *handles()[i];
    }

    return centroid * 0.25;
}

bool PerspectiveEllipseAssistant::quad(QPolygonF& poly) const
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

bool PerspectiveEllipseAssistant::getTransform(QPolygonF& poly, QTransform& transform) const
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
    
    
//     // Approach 1: This is from the math that I derived. I used the touch points of the ellipse and the four point cage (pt1, pt2, pt3, pt4) 
       // and the two corners of the four point cage (ptA, ptB) to solve and find the coefficients of the equation of the ellipse. 
    
       // There are 6 coefficients, namely a, b, c, d, e, f. But since a can never be zero, I divided the equation by a, and the number of
       // unknowns reduce to 5, namely b/a, c/a, d/a, e/a, f/a, which are denoted here by x(0), x(1), x(2), x(3), x(4) respectively. 
    
       // Using these coefficients, I evaluate the extreme points of the major axis of the ellipse, denoted by QPointF left, right. 
       // Also evaluated top in the same manner. 
    
       // I had stored top, left, right in m_cachedEllipsePoints. However, doing this, I didn't get the correct ellipse. Rather I got an ellipse
       // that disappears at certain positions, and changed shape and size on being translated. 
    
    
//     QPointF pt1 = m_cachedTransform.map(QPointF(0.5, 1.0));
//     QPointF pt2 = m_cachedTransform.map(QPointF(1.0, 0.5));
//     QPointF pt3 = m_cachedTransform.map(QPointF(0.5, 0.0));
//     QPointF pt4 = m_cachedTransform.map(QPointF(0.0, 0.5));
//     
//     QPointF ptA = m_cachedTransform.map(QPointF(0.0, 1.0));
//     QPointF ptB = m_cachedTransform.map(QPointF(1.0, 1.0));
//     
//     qDebug() << pt1 << pt2 << pt3 << pt4 << ptA << ptB;
//     
//     Eigen::MatrixXf A(5, 5);
//     A << (pt1.y() * (ptA.x() - ptB.x())), 0.0, (ptA.x() - ptB.x()), 0.0, 0.0, pt1.x() * pt1.y(), pt1.y() * pt1.y(), pt1.x(), pt1.y(), 1.0,
//                   pt2.x() * pt2.y(), pt2.y() * pt2.y(), pt2.x(), pt2.y(), 1.0,
//                   pt3.x() * pt3.y(), pt3.y() * pt3.y(), pt3.x(), pt3.y(), 1.0,
//                   pt4.x() * pt4.y(), pt4.y() * pt4.y(), pt4.x(), pt4.y(), 1.0;
//                   
//     Eigen::VectorXf b(5);
//     b << ptA.y() - ptB.y() - 2.0 * pt1.x() * (ptA.x() - ptB.x()), - pt1.x() * pt1.x(), - pt2.x() * pt2.x(),  - pt3.x() * pt3.x(), - pt4.x() * pt4.x();
//     
//     Eigen::VectorXf x = A.fullPivLu().solve(b);
//     
//     qDebug() << x(0) << x(1) << x(2) << x(3) << x(4);
//     QPointF center = QPointF((x(0)*x(3) - 2*x(1)*x(2))/(4*x(1) - x(0)*x(0)), (x(0)*x(2) - 2*x(3))/(4*x(1) - x(0)*x(0)));
//     
//     qreal xl = center.x() - qPow(qPow((2*x(0)*x(3) - 4*x(1)*x(2)), 2) + 4*(4*x(1) - x(0)*x(0))*(x(3)*x(3) - 4*x(1)*x(4)), 0.5)/(2*(4*x(1) - x(0)*x(0)));
//     QPointF left = QPointF(xl, (-x(0)*xl - x(3))/2*x(1));
//     
//     qreal xr = center.x() + qPow(qPow((2*x(0)*x(3) - 4*x(1)*x(2)), 2) + 4*(4*x(1) - x(0)*x(0))*(x(3)*x(3) - 4*x(1)*x(4)), 0.5)/(2*(4*x(1) - x(0)*x(0)));
//     QPointF right = QPointF(xr, (-x(0)*xr - x(3))/2*x(1));
//     
//     qreal yt = center.y() + qPow(qPow((2*x(0)*x(2) - 4*x(3)), 2) + 4*(4*x(1) - x(0)*x(0))*(x(3)*x(3) - 4*x(4)), 0.5)/(2*(4*x(1) - x(0)*x(0)));
//     QPointF top = QPointF((-x(0)*yt - x(2))/2, yt);
//     
//     qDebug() << center << left << right;
//     
//     m_cachedEllipsePoints[0] = left;
//     m_cachedEllipsePoints[1] = right;
//     m_cachedEllipsePoints[2] = top;
    

    // Apporach 2: Dmitry suggested I used the KisAlgebra2D::transformEllipse() to get the new transform after applying the old transform
    // to the old major and minor axis, which is the circle inscribed inside the unit square, having center as (0.5, 0.5) and radius = 0.5
    
    // However, since KisAlgebra2D::transformEllipse() only works for a circle centered at (0, 0), I had to take care of the offset since 
    // in my case the circle is centered at (0.5, 0.5)

    
    QPointF newAxes;
    QTransform newTransform;
    
    QTransform oldTransform = QTransform::fromTranslate(0.5, 0.5) * m_cachedTransform;

    std::tie(newAxes, newTransform) = KisAlgebra2D::transformEllipse(QPointF(0.5, 0.5), oldTransform);

    const QPointF p1 = (QPointF(newAxes.x(), 0));
    const QPointF p2 = (QPointF(-newAxes.x(), 0));
    const QPointF p3 = (QPointF(0, newAxes.y()));

    m_cachedEllipsePoints[0] = p1;
    m_cachedEllipsePoints[1] = p2;
    m_cachedEllipsePoints[2] = p3;
    
    // This is used later in drawAssistant() and drawCache() to set the transform: gc.setTransform(initialTransform * m_cachedEllipseTransform);
    m_cachedEllipseTransform = newTransform;
    m_cacheValid = true;
    return true;
}

bool PerspectiveEllipseAssistant::isAssistantComplete() const
{   
    return handles().size() >= 4;
}

PerspectiveEllipseAssistantFactory::PerspectiveEllipseAssistantFactory()
{
}

PerspectiveEllipseAssistantFactory::~PerspectiveEllipseAssistantFactory()
{
}

QString PerspectiveEllipseAssistantFactory::id() const
{
    return "perspective ellipse";
}

QString PerspectiveEllipseAssistantFactory::name() const
{
    return i18n("Perspective Ellipse");
}

KisPaintingAssistant* PerspectiveEllipseAssistantFactory::createPaintingAssistant() const
{
    return new PerspectiveEllipseAssistant;
}
