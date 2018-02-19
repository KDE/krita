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

#ifndef KISHANDLEPAINTERHELPER_H
#define KISHANDLEPAINTERHELPER_H

#include "kritaglobal_export.h"
#include "kis_algebra_2d.h"

#include <QPainter>
#include <KisHandleStyle.h>
#include "kis_painting_tweaks.h"

#include <KisHandleUtililtyTypes.h>

#include <boost/variant.hpp>

class QPainter;
class KoShape;
class KoViewConverter;

/**
 * @brief The KisHandlePainterHelper class is a special helper for
 *        painting handles around objects. It ensures the handlesare painted
 *        with the same size and line width whatever transformation is setup
 *        in the painter. The handles will also be rotated/skewed if the object
 *        itself has these transformations.
 *
 *        On construction it resets QPainter transformation and on destruction
 *        recovers it back.
 *
 * Please consider using KoShape::createHandlePainterHelper instead of direct
 * construction of the helper. This factory method will also apply the
 * transformations needed for a shape.
 */

class KRITAGLOBAL_EXPORT KisHandlePainterHelper
{
public:
    using Handle = KritaUtils::Handle;

public:
    KisHandlePainterHelper();

    /**
     * Creates the helper, initializes all the internal transformations and
     * *resets* the transformation of the painter.
     */
    KisHandlePainterHelper(QPainter *_painter,
                           const QTransform &localToViewTransform,
                           const QTransform &localToDocTransform,
                           qreal handleRadius);

    /**
     * Move c-tor. Used to create and return the helper from functions by-value.
     */
    KisHandlePainterHelper(KisHandlePainterHelper &&rhs);
    KisHandlePainterHelper(KisHandlePainterHelper &rhs) = delete;

    KisHandlePainterHelper& operator=(KisHandlePainterHelper &&rhs);
    KisHandlePainterHelper& operator=(KisHandlePainterHelper &rhs) = delete;

    /**
     * Restores the transformation of the painter
     */
    ~KisHandlePainterHelper();

    /**
     * Sets style used for painting the handles. Please use static methods of
     * KisHandleStyle to select predefined styles.
     */
    void setHandleStyle(const KisHandleStyle &style);

    /**
     * Draw an abstract handle \p handle, which might encapsulate either point or line
     */
    void drawHandle(const Handle &handle);

    /**
     * Calculate bounding box of an abstract handle \p handle, which can
     * encapsulate either point or line.
     */
    QRectF handleBoundingRectDoc(const Handle &handle) const;

    /**
     * Draws a handle rect with a custom \p radius at position \p center
     */
    void drawHandleRect(const QPointF &center, qreal radius);

    /**
     * Draws a handle circle with a custom \p radius at position \p center
     */
    void drawHandleCircle(const QPointF &center, qreal radius);

    /**
     * Optimized version of the drawing method for drawing handles of
     * predefined size
     */
    void drawHandleRect(const QPointF &center);

    /**
     * Optimized version of the drawing method for drawing handles of
     * predefined size
     */
    void drawHandleCircle(const QPointF &center);

    /**
     * Optimized version of the drawing method for drawing handles of
     * predefined size
     */
    void drawHandleSmallCircle(const QPointF &center);

    /**
     * Draw a rotated handle representing the gradient handle
     */
    void drawGradientHandle(const QPointF &center, qreal radius);

    /**
     * Draw a rotated handle representing the gradient handle
     */
    void drawGradientHandle(const QPointF &center);

    /**
     * Draw a special handle representing the center of the gradient
     */
    void drawGradientCrossHandle(const QPointF &center, qreal radius);

    /**
     * Draw an arrow representing gradient position
     */
    void drawGradientArrow(const QPointF &start, const QPointF &end, qreal radius);

    /**
     * Draw a line showing the bounding box of the selection
     */
    void drawRubberLine(const QPolygonF &poly);

    /**
     * Draw a line connecting two points
     */
    void drawConnectionLine(const QLineF &line);

    /**
     * Draw a line connecting two points
     */
    void drawConnectionLine(const QPointF &p1, const QPointF &p2);

    /**
     * Draw an arbitrary path
     */
    void drawPath(const QPainterPath &path);

private:

    QPainterPath calculatePointHandlePath(KritaUtils::HandlePointType type) const;
    QPainterPath calculateLineHandlePath(const Handle::LineHandle &handle) const;
    QPainterPath calculateArrow(const QPointF &pos, const QPointF &from, qreal radius) const;

    void drawPathImpl(const QPainterPath &path);
    void strokePathImpl(const QPainterPath &path);
    QRectF pathBoundingRectDocImpl(const QPainterPath &poly) const;

    /**
     * Draw a single arrow with the tip at position \p pos, directed from \p from,
     * of size \p radius.
     */
    void drawArrow(const QPointF &pos, const QPointF &from, qreal radius);

    void init();

private:
    KisPaintingTweaks::TransformSaver m_transformSaver;

    QPainter *m_painter = 0;
    QTransform m_painterTransform;
    QTransform m_localToDocTransform;
    qreal m_handleRadius = 0.0;
    KisAlgebra2D::DecomposedMatix m_decomposedMatrix;
    QTransform m_handleTransform;
    QPolygonF m_handlePolygon;
    KisHandleStyle m_handleStyle;
};

#endif // KISHANDLEPAINTERHELPER_H
