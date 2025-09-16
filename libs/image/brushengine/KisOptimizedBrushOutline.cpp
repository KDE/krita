/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOptimizedBrushOutline.h"

#include <QPainterPath>
#include <QTransform>
#include <kis_algebra_2d.h>

KisOptimizedBrushOutline::KisOptimizedBrushOutline()
{
}

KisOptimizedBrushOutline::KisOptimizedBrushOutline(const QPainterPath &path, const std::optional<QRectF> &bounds)
    : KisOptimizedBrushOutline(path.toSubpathPolygons().toVector(), bounds)
{
    // storing in a form of a QVector is much more efficient
    // than in a QList
}

KisOptimizedBrushOutline::KisOptimizedBrushOutline(const QVector<QPolygonF> &subpaths, const std::optional<QRectF> &bounds)
    : m_subpaths(subpaths)
    , m_explicitBounds(bounds)
{
}

void KisOptimizedBrushOutline::map(const QTransform &t)
{
    m_transform *= t;
    m_cachedBoundingRect = QRectF();
}

KisOptimizedBrushOutline KisOptimizedBrushOutline::mapped(const QTransform &t) const
{
    KisOptimizedBrushOutline result(*this);
    result.map(t);
    return result;
}

KisOptimizedBrushOutline::const_iterator KisOptimizedBrushOutline::begin() const
{
    return const_iterator(this, 0);
}

KisOptimizedBrushOutline::const_iterator KisOptimizedBrushOutline::end() const
{
    return const_iterator(this, m_subpaths.size() + m_additionalDecorations.size());
}

QRectF KisOptimizedBrushOutline::boundingRect() const
{
    if (!m_cachedBoundingRect.isNull()) return m_cachedBoundingRect;

    /**
     * We don't use normal begin()/end() iteration here,
     * because it makes too many allocations for the polygons.
     * Instead we calculate the bounding rect by mere iteration
     * over points.
     */

    QRectF result;
    bool resultInitialized = false;

    if (m_explicitBounds && !m_explicitBounds->isEmpty()) {
        result = m_transform.mapRect(*m_explicitBounds);
        resultInitialized = true;
    }

    for (auto polyIt = m_subpaths.cbegin(); polyIt != m_subpaths.cend(); ++polyIt) {
        /**
         * This is a highly optimized way to accumulate a rect from a
         * set of points:
         *
         * 1) `QRectF::isEmpty()` is expensive, so use `resultInitialized` instead
         * 2) Use `KisAlgebra2D::accumulateBoundsNonEmpty` to avoid calling `isEmpty()`
         */


        auto it = polyIt->cbegin();

        if (!resultInitialized && it != polyIt->cend()) {
            KisAlgebra2D::Private::resetEmptyRectangle(m_transform.map(*it), &result);
            resultInitialized = true;
            ++it;
        }

        for (; it != polyIt->cend(); ++it) {
            KisAlgebra2D::accumulateBoundsNonEmpty(m_transform.map(*it), &result);
        }
    }

    for (auto polyIt = m_additionalDecorations.cbegin(); polyIt != m_additionalDecorations.cend(); ++polyIt) {
        auto it = polyIt->cbegin();

        if (!resultInitialized && it != polyIt->cend()) {
            KisAlgebra2D::Private::resetEmptyRectangle(m_transform.map(*it), &result);
            resultInitialized = true;
            ++it;
        }

        for (; it != polyIt->cend(); ++it) {
            KisAlgebra2D::accumulateBoundsNonEmpty(m_transform.map(*it), &result);
        }
    }

    m_cachedBoundingRect = result;

    return result;
}

bool KisOptimizedBrushOutline::isEmpty() const
{
    return begin() == end();
}

void KisOptimizedBrushOutline::addRect(const QRectF &rc)
{
    QPainterPath path;
    path.addRect(rc);
    addPath(path);
}

void KisOptimizedBrushOutline::addEllipse(const QPointF &center, qreal rx, qreal ry)
{
    QPainterPath path;
    path.addEllipse(center, rx, ry);
    addPath(path);
}

void KisOptimizedBrushOutline::addPath(const QPainterPath &path)
{
    addPath(KisOptimizedBrushOutline(path));
}

void KisOptimizedBrushOutline::addPath(const KisOptimizedBrushOutline &path)
{
    const QTransform invertedTransform = path.m_transform * m_transform.inverted();

    m_additionalDecorations.reserve(m_additionalDecorations.size() +
                                    path.m_subpaths.size() +
                                    path.m_additionalDecorations.size());

    for (auto it = path.m_subpaths.cbegin(); it != path.m_subpaths.cend(); ++it) {
        m_additionalDecorations.append(invertedTransform.map(*it));
    }

    for (auto it = path.m_additionalDecorations.cbegin(); it != path.m_additionalDecorations.cend(); ++it) {
        m_additionalDecorations.append(invertedTransform.map(*it));
    }

    m_cachedBoundingRect = QRectF();
}

void KisOptimizedBrushOutline::translate(qreal tx, qreal ty)
{
    map(QTransform::fromTranslate(tx, ty));
}

void KisOptimizedBrushOutline::translate(const QPointF &offset)
{
    translate(offset.x(), offset.y());
}

QPolygonF KisOptimizedBrushOutline::const_iterator::dereference() const
{
    int index = m_index;

    if (index < m_outline->m_subpaths.size()) {
        return m_outline->m_transform.map(m_outline->m_subpaths.at(index));
    }

    index -= m_outline->m_subpaths.size();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(index >= 0, QPolygonF());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(index < m_outline->m_additionalDecorations.size(), QPolygonF());

    return m_outline->m_transform.map(m_outline->m_additionalDecorations.at(index));
}
