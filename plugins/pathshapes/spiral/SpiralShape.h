/* This file is part of the KDE project
   Copyright (C) 2007 Rob Buis <buis@kde.org>

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

#ifndef KOSPIRALSHAPE_H
#define KOSPIRALSHAPE_H

#include "KoParameterShape.h"

#define SpiralShapeId "SpiralShape"

/**
 * This class adds support for the spiral
 * shape.
 */
class SpiralShape : public KoParameterShape
{
public:
    /// the possible spiral types
    enum SpiralType
    {
        Curve = 0,   ///< spiral uses curves
        Line = 1    ///< spiral uses lines
    };

    SpiralShape();
    ~SpiralShape();

    void setSize(const QSizeF &newSize);
    virtual QPointF normalize();

    /**
     * Sets the type of the spiral.
     * @param type the new spiral type
     */
    void setType(SpiralType type);

    /// Returns the actual spiral type
    SpiralType type() const;

    /**
     * Sets the fade parameter of the spiral.
     * @param angle the new start angle in degree
     */
    void setFade(qreal fade);

    /// Returns the actual fade parameter
    qreal fade() const;

    bool clockWise() const;
    void setClockWise(bool clockwise);

    /// reimplemented
    virtual QString pathShapeId() const;

protected:
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void updatePath(const QSizeF &size);
    void createPath(const QSizeF &size);

private:
    void updateKindHandle();
    void updateAngleHandles();

    // fade parameter
    qreal m_fade;
    // angle for modifying the kind in radiant
    qreal m_kindAngle;
    // the center of the spiral
    QPointF m_center;
    // the radii of the spiral
    QPointF m_radii;
    // the actual spiral type
    SpiralType m_type;
    //
    bool m_clockwise;

    KoSubpath m_points;
};

#endif /* KOSPIRALSHAPE_H */

