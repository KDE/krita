/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "PerspectiveAssistant.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_algebra_2d.h>
#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_dom_utils.h>

#include "PerspectiveBasedAssistantHelper.h"

#include <math.h>
#include <limits>

PerspectiveAssistant::PerspectiveAssistant(QObject *parent)
    : KisAbstractPerspectiveGrid(parent)
    , KisPaintingAssistant("perspective", i18n("Perspective assistant"))
{
}

PerspectiveAssistant::PerspectiveAssistant(const PerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisAbstractPerspectiveGrid(rhs.parent())
    , KisPaintingAssistant(rhs, handleMap)
    , m_subdivisions(rhs.m_subdivisions)
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

QPointF PerspectiveAssistant::project(const QPointF& pt, const QPointF& strokeBegin, const bool alwaysStartAnew, qreal moveThresholdPt)
{
    const static QPointF nullPoint(std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());

    Q_ASSERT(isAssistantComplete());

    if (alwaysStartAnew || m_snapLine.isNull()) {
        QPolygonF poly;
        QTransform transform;

        if (!getTransform(poly, transform)) {
            return nullPoint;
        }

        if (!poly.containsPoint(strokeBegin, Qt::OddEvenFill)) {
            return nullPoint; // avoid problems with multiple assistants: only snap if starting in the grid
        }

        if (KisAlgebra2D::norm(pt - strokeBegin) < moveThresholdPt) {
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
        m_snapLine = KisAlgebra2D::pointToLineDistSquared(pt, verticalLine) < KisAlgebra2D::pointToLineDistSquared(pt, horizontalLine) ? verticalLine : horizontalLine;
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

QPointF PerspectiveAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/, qreal moveThresholdPt)
{
    return project(pt, strokeBegin, false, moveThresholdPt);
}

void PerspectiveAssistant::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    point = project(point, strokeBegin, true, 0.0);
}

void PerspectiveAssistant::endStroke()
{
    m_snapLine = QLineF();
    KisPaintingAssistant::endStroke();
}

bool PerspectiveAssistant::contains(const QPointF& pt) const
{
    QPolygonF poly;
    if (!PerspectiveBasedAssistantHelper::getTetragon(handles(), isAssistantComplete(), poly)) return false;
    return poly.containsPoint(pt, Qt::OddEvenFill);
}

qreal PerspectiveAssistant::distance(const QPointF& pt) const
{
    return PerspectiveBasedAssistantHelper::distanceInGrid(m_cache, pt);
}

bool PerspectiveAssistant::isActive() const
{
    return isSnappingActive();
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
        if (m_cache.vanishingPoint1) {
            drawX(gc, initialTransform.map(m_cache.vanishingPoint1.get()));
        }
        if (m_cache.vanishingPoint2) {
            drawX(gc, initialTransform.map(m_cache.vanishingPoint2.get()));
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

        if (m_followBrushPosition && m_adjustedPositionValid) {
            mousePos = initialTransform.map(m_adjustedBrushPosition);
        }

        //figure out if point is in the perspective grid
        QPointF intersectTransformed(0, 0); // dummy for holding transformed intersection so the code is more readable.

        if (poly.containsPoint(initialTransform.inverted().map(mousePos), Qt::OddEvenFill)==true){
            // check if the lines aren't parallel to each other to avoid calculation errors in the intersection calculation (bug 345754)//
            if (fmod(QLineF(poly[0], poly[1]).angle(), 180.0)>=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)+2.0 || fmod(QLineF(poly[0], poly[1]).angle(), 180.0)<=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)-2.0) {
                if (QLineF(poly[0], poly[1]).intersect(QLineF(poly[2], poly[3]), &intersection) != QLineF::NoIntersection) {
                    intersectTransformed = initialTransform.map(intersection);
                    snapLine = QLineF(intersectTransformed, mousePos);
                    KisAlgebra2D::intersectLineRect(snapLine, viewport, true);
                    bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());

                    if (bounds.contains(intersectTransformed.toPoint())){
                        path2.moveTo(intersectTransformed);
                        path2.lineTo(snapLine.p2());
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
                    KisAlgebra2D::intersectLineRect(snapLine, viewport, true);
                    bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());
                    QPainterPath path;

                    if (bounds.contains(intersectTransformed.toPoint())){
                        path2.moveTo(intersectTransformed);
                        path2.lineTo(snapLine.p2());
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



    // draw the grid lines themselves
    gc.setTransform(converter->documentToWidgetTransform());

    if (assistantVisible) {
        // getTransform was checked before but what if the preview wasn't visible etc., and we need a return value here too
        if (!getTransform(poly, transform)) {
            // color red for an invalid transform, but not for an incomplete one
            if(isAssistantComplete()) {
                QPainterPath path;
                // that will create a triangle with a point inside connected to all vertices of the triangle
                path.addPolygon(PerspectiveBasedAssistantHelper::getAllConnectedTetragon(handles()));
                drawError(gc, path);
            } else {
                QPainterPath path;
                path.addPolygon(poly);
                drawPath(gc, path, isSnappingActive());
            }
        } else {
            gc.setPen(QColor(0, 0, 0, 125));
            gc.setTransform(transform, true);
            QPainterPath path;
            qreal step = 1.0 / subdivisions();
            
            for (int y = 0; y <= subdivisions(); ++y)
            {
                QLineF line = QLineF(QPointF(0.0, y * step), QPointF(1.0, y * step));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }
            for (int x = 0; x <= subdivisions(); ++x)
            {
                QLineF line = QLineF(QPointF(x * step, 0.0), QPointF(x * step, 1.0));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }

            drawPath(gc, path, isSnappingActive());
        }
    }
    //


    gc.restore();

    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached,canvas, assistantVisible, previewVisible);
}

void PerspectiveAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
    Q_UNUSED(assistantVisible);
}

QPointF PerspectiveAssistant::getDefaultEditorPosition() const
{
    QPointF centroid(0, 0);
    for (int i = 0; i < 4; ++i) {
        centroid += *handles()[i];
    }

    return centroid * 0.25;
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

    if (!PerspectiveBasedAssistantHelper::getTetragon(handles(), isAssistantComplete(), poly)) {
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
    PerspectiveBasedAssistantHelper::updateCacheData(m_cache, poly);
    m_cacheValid = true;
    return true;
}

bool PerspectiveAssistant::isAssistantComplete() const
{
    return handles().size() >= 4; // specify 4 corners to make assistant complete
}

int PerspectiveAssistant::subdivisions() const {
    return m_subdivisions;
}

void PerspectiveAssistant::setSubdivisions(int subdivisions) {
    if (subdivisions < 1) m_subdivisions = 1;
    else m_subdivisions = subdivisions;
}

void PerspectiveAssistant::saveCustomXml(QXmlStreamWriter *xml) {
    if (xml) {
        xml->writeStartElement("subdivisions");
        xml->writeAttribute("value", KisDomUtils::toString(subdivisions()));
        xml->writeEndElement();
    }
}

bool PerspectiveAssistant::loadCustomXml(QXmlStreamReader *xml) {
    if (xml && xml->name() == "subdivisions") {
        setSubdivisions(KisDomUtils::toInt(xml->attributes().value("value").toString()));
    }
    return true;
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
