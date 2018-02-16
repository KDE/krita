/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisHandlePainterHelper.h"

#include <QPainter>
#include "kis_algebra_2d.h"
#include "kis_painting_tweaks.h"

using KisPaintingTweaks::PenBrushSaver;
using KisPaintingTweaks::TransformSaver;

KisHandlePainterHelper::KisHandlePainterHelper()
    : m_transformSaver(0, QTransform(), TransformSaver::allow_noop),
      m_painter(0),
      m_handleRadius(0)
{
}

KisHandlePainterHelper::KisHandlePainterHelper(QPainter *painter,
                                               const QTransform &localToViewTransform,
                                               const QTransform &localToDocTransform,
                                               qreal handleRadius)
    : m_transformSaver(painter, QTransform(), TransformSaver::allow_noop),
      m_painter(painter),
      m_painterTransform(localToViewTransform),
      m_localToDocTransform(localToDocTransform),
      m_handleRadius(handleRadius),
      m_decomposedMatrix(localToViewTransform)
{
    init();
}

KisHandlePainterHelper::KisHandlePainterHelper(KisHandlePainterHelper &&rhs)
    : m_transformSaver(std::move(rhs.m_transformSaver)),
      m_painter(rhs.m_painter),
      m_painterTransform(rhs.m_painterTransform),
      m_localToDocTransform(rhs.m_localToDocTransform),
      m_handleRadius(rhs.m_handleRadius),
      m_decomposedMatrix(rhs.m_decomposedMatrix),
      m_handleTransform(rhs.m_handleTransform),
      m_handlePolygon(rhs.m_handlePolygon),
      m_handleStyle(rhs.m_handleStyle)
{
    // disable the source helper
    rhs.m_painter = 0;
}

KisHandlePainterHelper& KisHandlePainterHelper::operator=(KisHandlePainterHelper &&rhs)
{
    m_transformSaver = std::move(rhs.m_transformSaver);
    m_painter = rhs.m_painter;
    m_painterTransform = rhs.m_painterTransform;
    m_localToDocTransform = rhs.m_localToDocTransform;
    m_handleRadius = rhs.m_handleRadius;
    m_decomposedMatrix = rhs.m_decomposedMatrix;
    m_handleTransform = rhs.m_handleTransform;
    m_handlePolygon = rhs.m_handlePolygon;
    m_handleStyle = rhs.m_handleStyle;

    // disable the source helper
    rhs.m_painter = 0;

    return *this;
}

void KisHandlePainterHelper::init()
{
    m_handleStyle = KisHandleStyle::inheritStyle();
    m_handleTransform = m_decomposedMatrix.shearTransform() * m_decomposedMatrix.rotateTransform();

    if (m_handleRadius > 0.0) {
        const QRectF handleRect(-m_handleRadius, -m_handleRadius, 2 * m_handleRadius, 2 * m_handleRadius);
        m_handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    }
}

KisHandlePainterHelper::~KisHandlePainterHelper()
{
}

void KisHandlePainterHelper::setHandleStyle(const KisHandleStyle &style)
{
    m_handleStyle = style;
}

QPainterPath KisHandlePainterHelper::calculatePointHandlePath(KisHandlePainterHelper::Type type) const
{
    auto rectPath = [] (qreal radius) {
        QRectF rect(-radius, -radius, 2 * radius, 2 * radius);
        QPainterPath path;
        path.addRect(rect);
        return path;
    };

    auto circlePath = [] (qreal radius) {
        QRectF rect(-radius, -radius, 2 * radius, 2 * radius);
        QPainterPath path;
        path.addEllipse(rect);
        return path;
    };

    auto diamondPolygon = [] (qreal radius) {
        QPolygonF poly;

        poly << QPointF(-radius, 0);
        poly << QPointF(0, radius);
        poly << QPointF(radius, 0);
        poly << QPointF(0, -radius);
        poly << QPointF(-radius, 0);

        QPainterPath path;
        path.addPolygon(poly);
        return path;
    };

    QPainterPath path;

    switch (type) {
    case Invalid:
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "invalid handle type");
        break;
    case Rect:
        path = m_handleTransform.map(rectPath(m_handleRadius));
        break;
    case Circle:
        path = circlePath(m_handleRadius);
        break;
    case SmallCircle:
        path = circlePath(0.7 * m_handleRadius);
        break;
    case Diamond:
        path = m_handleTransform.map(diamondPolygon(1.41 * m_handleRadius));
        break;
    case GradientDiamond:
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "not implemented");
        break;
    case GradientCross:
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "not implemented");
        break;
    }

    return path;
}

void KisHandlePainterHelper::drawPathImpl(const QPainterPath &path)
{
    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPath(path);
    }
}

QRectF KisHandlePainterHelper::pathBoundingRectDocImpl(const QPainterPath &path) const
{
    QPainterPath handlePath(path);

    // todo: cache transform!
    const QTransform viewToDoc = m_painterTransform.inverted() * m_localToDocTransform;
    handlePath = viewToDoc.map(handlePath);

    return kisGrowRect(handlePath.boundingRect(), 4.0);
}


void KisHandlePainterHelper::drawHandle(KisHandlePainterHelper::Type type, const QPointF &pos)
{
    QPainterPath handlePath = calculatePointHandlePath(type);
    handlePath.translate(m_painterTransform.map(pos));

    drawPathImpl(handlePath);
}

QRectF KisHandlePainterHelper::handleBoundingRectDoc(KisHandlePainterHelper::Type type, const QPointF &pos) const
{
    QPainterPath handlePath = calculatePointHandlePath(type);
    handlePath.translate(m_painterTransform.map(pos));

    return pathBoundingRectDocImpl(handlePath);
}

void KisHandlePainterHelper::drawHandle(KisHandlePainterHelper::LineType type, const QPointF &p1, const QPointF &p2)
{
    QPainterPath path;

    switch (type) {
    case ConnectionLine:
        path.moveTo(p1);
        path.lineTo(p2);
        break;
    }

    path = m_painterTransform.map(path);

    drawPathImpl(path);
}

QRectF KisHandlePainterHelper::handleBoundingRectDoc(KisHandlePainterHelper::LineType type, QPointF &p1, const QPointF &p2) const
{
    QPainterPath path;

    switch (type) {
    case ConnectionLine:
        path.moveTo(p1);
        path.lineTo(p2);
        break;
    }

    path = m_painterTransform.map(path);
    return pathBoundingRectDocImpl(path);
}

void KisHandlePainterHelper::drawHandle(const KisHandlePainterHelper::Handle &handle)
{
    struct Visitor : public boost::static_visitor<>
    {
        Visitor(KisHandlePainterHelper *_q) : q(_q) {}

        void operator()(Handle::PointHandle handle) const {
            q->drawHandle(handle.type, handle.pos);
        }
        void operator()(Handle::LineHandle handle) const {
            q->drawHandle(handle.type, handle.p1, handle.p2);
        }

        KisHandlePainterHelper *q;
    };

    boost::apply_visitor(Visitor(this), handle.value);
}

QRectF KisHandlePainterHelper::handleBoundingRectDoc(const KisHandlePainterHelper::Handle &handle) const
{
    struct Visitor : public boost::static_visitor<QRectF>
    {
        Visitor(const KisHandlePainterHelper *_q) : q(_q) {}

        QRectF operator()(Handle::PointHandle handle) const {
            return q->handleBoundingRectDoc(handle.type, handle.pos);
        }
        QRectF operator()(Handle::LineHandle handle) const {
            return q->handleBoundingRectDoc(handle.type, handle.p1, handle.p2);
        }

        const KisHandlePainterHelper *q;
    };

    return boost::apply_visitor(Visitor(this), handle.value);
}

void KisHandlePainterHelper::drawHandleRect(const QPointF &center, qreal radius) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QRectF handleRect(-radius, -radius, 2 * radius, 2 * radius);
    QPolygonF handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    handlePolygon.translate(m_painterTransform.map(center));

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPolygon(handlePolygon);
    }
}

void KisHandlePainterHelper::drawHandleCircle(const QPointF &center, qreal radius) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QRectF handleRect(-radius, -radius, 2 * radius, 2 * radius);
    handleRect.translate(m_painterTransform.map(center));

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawEllipse(handleRect);
    }
}

void KisHandlePainterHelper::drawHandleCircle(const QPointF &center)
{
    drawHandleCircle(center, m_handleRadius);
}

void KisHandlePainterHelper::drawHandleSmallCircle(const QPointF &center)
{
    drawHandleCircle(center, 0.7 * m_handleRadius);
}

void KisHandlePainterHelper::drawHandleRect(const QPointF &center) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);
    QPolygonF paintingPolygon = m_handlePolygon.translated(m_painterTransform.map(center));

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPolygon(paintingPolygon);
    }
}

void KisHandlePainterHelper::drawGradientHandle(const QPointF &center, qreal radius) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QPolygonF handlePolygon;

    handlePolygon << QPointF(-radius, 0);
    handlePolygon << QPointF(0, radius);
    handlePolygon << QPointF(radius, 0);
    handlePolygon << QPointF(0, -radius);

    handlePolygon = m_handleTransform.map(handlePolygon);
    handlePolygon.translate(m_painterTransform.map(center));

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPolygon(handlePolygon);
    }
}

void KisHandlePainterHelper::drawGradientHandle(const QPointF &center)
{
    drawGradientHandle(center, 1.41 * m_handleRadius);
}

void KisHandlePainterHelper::drawGradientCrossHandle(const QPointF &center, qreal radius) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    { // Draw a cross
        QPainterPath p;
        p.moveTo(-radius, -radius);
        p.lineTo(radius, radius);
        p.moveTo(radius, -radius);
        p.lineTo(-radius, radius);

        p = m_handleTransform.map(p);
        p.translate(m_painterTransform.map(center));

        Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
            PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
            m_painter->drawPath(p);
        }
    }

    { // Draw a square
        const qreal halfRadius = 0.5 * radius;

        QPolygonF handlePolygon;
        handlePolygon << QPointF(-halfRadius, 0);
        handlePolygon << QPointF(0, halfRadius);
        handlePolygon << QPointF(halfRadius, 0);
        handlePolygon << QPointF(0, -halfRadius);

        handlePolygon = m_handleTransform.map(handlePolygon);
        handlePolygon.translate(m_painterTransform.map(center));

        Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
            PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
            m_painter->drawPolygon(handlePolygon);
        }
    }
}

void KisHandlePainterHelper::drawArrow(const QPointF &pos, const QPointF &from, qreal radius)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QPainterPath p;

    QLineF line(pos, from);
    line.setLength(radius);

    QPointF norm = KisAlgebra2D::leftUnitNormal(pos - from);
    norm *= 0.34 * radius;

    p.moveTo(line.p2() + norm);
    p.lineTo(line.p1());
    p.lineTo(line.p2() - norm);

    p.translate(-pos);

    p = m_handleTransform.map(p).translated(m_painterTransform.map(pos));

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPath(p);
    }
}

void KisHandlePainterHelper::drawGradientArrow(const QPointF &start, const QPointF &end, qreal radius)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QPainterPath p;
    p.moveTo(start);
    p.lineTo(end);
    p = m_painterTransform.map(p);

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.lineIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPath(p);
    }

    const qreal length = kisDistance(start, end);
    const QPointF diff = end - start;

    if (length > 5 * radius) {
        drawArrow(start + 0.33 * diff, start, radius);
        drawArrow(start + 0.66 * diff, start, radius);
    } else if (length > 3 * radius) {
        drawArrow(start + 0.5 * diff, start, radius);
    }
}

void KisHandlePainterHelper::drawRubberLine(const QPolygonF &poly) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QPolygonF paintingPolygon = m_painterTransform.map(poly);

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.lineIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPolygon(paintingPolygon);
    }
}

void KisHandlePainterHelper::drawConnectionLine(const QLineF &line)
{
    drawConnectionLine(line.p1(), line.p2());
}

void KisHandlePainterHelper::drawConnectionLine(const QPointF &p1, const QPointF &p2)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QPointF realP1 = m_painterTransform.map(p1);
    QPointF realP2 = m_painterTransform.map(p2);

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.lineIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawLine(realP1, realP2);
    }
}

void KisHandlePainterHelper::drawPath(const QPainterPath &path)
{
    const QPainterPath realPath = m_painterTransform.map(path);

    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.lineIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPath(realPath);
    }
}
