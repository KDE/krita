/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    ~RectangleShape();

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
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /// reimplemented
    virtual QString pathShapeId() const;

    /// reimplemented from SvgShape
    virtual bool saveSvg(SvgSavingContext &context);

    /// reimplemented from SvgShape
    virtual bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context);

protected:

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void updatePath(const QSizeF &size);
    void createPoints(int requiredPointCount);
    void updateHandles();

private:
    qreal m_cornerRadiusX; ///< in percent of half of the rectangle width (a number between 0 and 100)
    qreal m_cornerRadiusY; ///< in percent of half of the rectangle height (a number between 0 and 100)
};

#endif /* KORECTANGLESHAPE_H */

