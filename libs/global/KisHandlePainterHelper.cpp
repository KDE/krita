/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisHandlePainterHelper.h"

#include <QPainter>
#include <QPainterPath>
#include "kis_algebra_2d.h"
#include "kis_painting_tweaks.h"

using KisPaintingTweaks::PenBrushSaver;

KisHandlePainterHelper::KisHandlePainterHelper(QPainter *_painter, qreal handleRadius)
    : m_painter(_painter),
      m_originalPainterTransform(m_painter->transform()),
      m_painterTransform(m_painter->transform()),
      m_handleRadius(handleRadius),
      m_decomposedMatrix(m_painterTransform)
{
    init();
}

KisHandlePainterHelper::KisHandlePainterHelper(QPainter *_painter, const QTransform &originalPainterTransform, qreal handleRadius)
    : m_painter(_painter),
      m_originalPainterTransform(originalPainterTransform),
      m_painterTransform(m_painter->transform()),
      m_handleRadius(handleRadius),
      m_decomposedMatrix(m_painterTransform)
{
    init();
}

KisHandlePainterHelper::KisHandlePainterHelper(KisHandlePainterHelper &&rhs)
    : m_painter(rhs.m_painter),
      m_originalPainterTransform(rhs.m_originalPainterTransform),
      m_painterTransform(rhs.m_painterTransform),
      m_handleRadius(rhs.m_handleRadius),
      m_decomposedMatrix(rhs.m_decomposedMatrix),
      m_handleTransform(rhs.m_handleTransform),
      m_handlePolygon(rhs.m_handlePolygon),
      m_handleStyle(rhs.m_handleStyle)
{
    // disable the source helper
    rhs.m_painter = 0;
}

void KisHandlePainterHelper::init()
{
    m_handleStyle = KisHandleStyle::inheritStyle();

    m_painter->setTransform(QTransform());
    m_handleTransform = m_decomposedMatrix.shearTransform() * m_decomposedMatrix.rotateTransform();

    if (m_handleRadius > 0.0) {
        const QRectF handleRect(-m_handleRadius, -m_handleRadius, 2 * m_handleRadius, 2 * m_handleRadius);
        m_handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    }
}

KisHandlePainterHelper::~KisHandlePainterHelper() {
    if (m_painter) {
        m_painter->setTransform(m_originalPainterTransform);
    }
}

void KisHandlePainterHelper::setHandleStyle(const KisHandleStyle &style)
{
    m_handleStyle = style;
}

void KisHandlePainterHelper::drawHandleRect(const QPointF &center, qreal radius, QPoint offset = QPoint(0,0))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QRectF handleRect(-radius, -radius, 2 * radius, 2 * radius);
    QPolygonF handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    handlePolygon.translate(m_painterTransform.map(center));

    handlePolygon.translate(offset);

    const QPen originalPen = m_painter->pen();

    // temporarily set the pen width to 2 to avoid pixel shifting dropping pixels the border
    QPen *tempPen = new QPen(m_painter->pen());
    tempPen->setWidth(4);
    const QPen customPen = *tempPen;
    m_painter->setPen(customPen);


    Q_FOREACH (KisHandleStyle::IterationStyle it, m_handleStyle.handleIterations) {
        PenBrushSaver saver(it.isValid ? m_painter : 0, it.stylePair, PenBrushSaver::allow_noop);
        m_painter->drawPolygon(handlePolygon);
    }

    m_painter->setPen(originalPen);
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

void KisHandlePainterHelper::drawPixmap(const QPixmap &pixmap, QPointF position, int size, QRectF sourceRect)
{
    QPointF handlePolygon = m_painterTransform.map(position);

    QPoint offsetPosition(0, 40);
    handlePolygon += offsetPosition;

    handlePolygon -= QPointF(size*0.5,size*0.5);

    m_painter->drawPixmap(QRect(handlePolygon.x(), handlePolygon.y(),
                                size, size),
                                pixmap,
                                sourceRect);
}

void KisHandlePainterHelper::fillHandleRect(const QPointF &center, qreal radius, QColor fillColor, QPoint offset = QPoint(0,0))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_painter);

    QRectF handleRect(-radius, -radius, 2 * radius, 2 * radius);
    QPolygonF handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    handlePolygon.translate(m_painterTransform.map(center));

    QPainterPath painterPath;
    painterPath.addPolygon(handlePolygon);

    // offset that happens after zoom transform. This means the offset will be the same, no matter the zoom level
    // this is good for UI elements that need to be below the bounding box
    painterPath.translate(offset);

    const QPainterPath pathToSend = painterPath;
    const QBrush brushStyle(fillColor);
    m_painter->fillPath(pathToSend, brushStyle);
}
