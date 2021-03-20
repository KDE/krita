/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KORECTANGLESHAPE_H
#define KORECTANGLESHAPE_H

#include "KoParameterShape.h"
#include <SvgShape.h>

#define RectangleShapeId "RectangleShape"

/**
 * The RectangleShape is a shape that represents a rectangle.
 * The rectangle can have rounded corners, with different corner
 * radii in x- and y-direction.
 */
class RectangleShape : public KoParameterShape, public SvgShape
{
public:
    RectangleShape();
    ~RectangleShape() override;

    KoShape* cloneShape() const override;

    /// Returns the corner radius in x-direction
    qreal cornerRadiusX() const;

    /**
     * Sets the corner radius in x-direction.
     *
     * The corner x-radius is a percent value (a number between 0 and 100)
     * of the half size of the rectangles width.
     *
     * @param radius the new corner radius in x-direction
     */
    void setCornerRadiusX(qreal radius);

    /// Returns the corner radius in y-direction
    qreal cornerRadiusY() const;

    /**
     * Sets the corner radius in y-direction.
     *
     * The corner y-radius is a percent value (a number between 0 and 100)
     * of the half size of the rectangles height.
     *
     * @param radius the new corner radius in y-direction
     */
    void setCornerRadiusY(qreal radius);
    /// reimplemented
    QString pathShapeId() const override;

    /// reimplemented from SvgShape
    bool saveSvg(SvgSavingContext &context) override;

    /// reimplemented from SvgShape
    bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context) override;

protected:
    RectangleShape(const RectangleShape &rhs);

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;
    void updatePath(const QSizeF &size) override;
    void createPoints(int requiredPointCount);
    void updateHandles();

private:
    qreal m_cornerRadiusX; ///< in percent of half of the rectangle width (a number between 0 and 100)
    qreal m_cornerRadiusY; ///< in percent of half of the rectangle height (a number between 0 and 100)
};

#endif /* KORECTANGLESHAPE_H */

