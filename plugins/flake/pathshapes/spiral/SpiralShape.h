/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    enum SpiralType {
        Curve = 0,   ///< spiral uses curves
        Line = 1    ///< spiral uses lines
    };

    SpiralShape();
    ~SpiralShape() override;

    KoShape* cloneShape() const override;

    void setSize(const QSizeF &newSize) override;
    QPointF normalize() override;

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
    QString pathShapeId() const override;

protected:
    SpiralShape(const SpiralShape &rhs);

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;
    void updatePath(const QSizeF &size) override;
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

