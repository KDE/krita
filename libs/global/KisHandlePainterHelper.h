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

class QPainter;

/**
 * @brief The KisHandlePainterHelper class is a special helper for
 *        painting handles around objects. It ensures the handlesare painted
 *        with the same size and line width whatever transformation is setup
 *        in the painter. The handles will also be rotated/skewed if the object
 *        itself has these transformations.
 *
 *        On construction it resets QPainter transformation and on destruction
 *        recovers it back.
 */

class KRITAGLOBAL_EXPORT KisHandlePainterHelper
{
public:

    /**
     * Creates the helper, initializes all the internal transformations and
     * *resets* the transformation of teh painter.
     */
    KisHandlePainterHelper(QPainter *_painter, qreal handleRadius = 0.0);

    /**
     * Restores the transformation of the painter
     */
    ~KisHandlePainterHelper();

    /**
     * Draws a handle rect with a custom \p radius at position \p center
     */
    void drawHandleRect(const QPointF &center, qreal radius);

    /**
     * Optimized version of the drawing method for drawing handles of
     * predefined size
     */
    void drawHandleRect(const QPointF &center);

    /**
     * Draw a rotated handle representing the gradient handle
     */
    void drawGradientHandle(const QPointF &center, qreal radius);

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

private:

    /**
     * Draw a single arrow with the tip at position \p pos, directed from \p from,
     * of size \p radius.
     */
    void drawArrow(const QPointF &pos, const QPointF &from, qreal radius);

private:
    QPainter *m_painter;
    QTransform m_painterTransform;
    KisAlgebra2D::DecomposedMatix m_decomposedMatrix;
    QTransform m_handleTransform;
    QPolygonF m_handlePolygon;
};

#endif // KISHANDLEPAINTERHELPER_H
