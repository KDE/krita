/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2008 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSTARSHAPE_H
#define KOSTARSHAPE_H

#include <array>
#include <KoParameterShape.h>

#define StarShapeId "StarShape"

/**
 * The star shape is a shape that can represent a star or
 * a regular polygon. There a some properties which can
 * be changed to control the appearance of the shape
 * like the number of corners, the inner/outer radius
 * and the corner roundness.
 */
class StarShape : public KoParameterShape
{
public:
    StarShape();
    ~StarShape() override;

    KoShape* cloneShape() const override;

    /**
     * Sets the number of corners.
     *
     * The minimum accepted number of corners is 3.
     * If the star is set to be convex (like a regular polygon),
     * the corner count equals the number of polygon points.
     * For a real star it represents the number of legs the star has.
     *
     * @param cornerCount the new number of corners
     */
    void setCornerCount(uint cornerCount);

    /// Returns the number of corners
    uint cornerCount() const;

    /**
     * Sets the radius of the base points.
     * The base radius has no meaning if the star is set convex.
     * @param baseRadius the new base radius
     */
    void setBaseRadius(qreal baseRadius);

    /// Returns the base radius
    qreal baseRadius() const;

    /**
     * Sets the radius of the tip points.
     * @param tipRadius the new tip radius
     */
    void setTipRadius(qreal tipRadius);

    /// Returns the tip radius
    qreal tipRadius() const;

    /**
     * Sets the roundness at the base points.
     *
     * A roundness value of zero disables the roundness.
     *
     * @param baseRoundness the new base roundness
     */
    void setBaseRoundness(qreal baseRoundness);

    /**
     * Sets the roundness at the tip points.
     *
     * A roundness value of zero disables the roundness.
     *
     * @param tipRoundness the new base roundness
     */
    void setTipRoundness(qreal tipRoundness);

    /**
     * Sets the star to be convex, looking like a polygon.
     * @param convex if true makes shape behave like regular polygon
     */
    void setConvex(bool convex);

    /// Returns if the star represents a regular polygon.
    bool convex() const;

    /**
     * Returns the star center point in shape coordinates.
     *
     * The star center is the weight center of the star and not necessarily
     * coincident with the shape center point.
     */
    QPointF starCenter() const;

    /// reimplemented
    void setSize(const QSizeF &newSize) override;
    /// reimplemented
    QString pathShapeId() const override;

protected:
    StarShape(const StarShape &rhs);

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;
    void updatePath(const QSizeF &size) override;
    /// recreates the path points when the corner count or convexity changes
    void createPoints(int requiredPointCount);

private:
    /// Computes the star center point from the inner points
    QPointF computeCenter() const;

    /// Returns the default offset angle in radian
    double defaultAngleRadian() const;

    /// the handle types
    enum Handles { tip = 0, base = 1 };

    uint m_cornerCount;    ///< number of corners
    std::array<qreal, 2> m_radius;    ///< the different radii
    std::array<qreal, 2> m_angles;    ///< the offset angles
    qreal m_zoomX;        ///< scaling in x
    qreal m_zoomY;        ///< scaling in y
    std::array<qreal, 2> m_roundness; ///< the roundness at the handles
    QPointF m_center;      ///< the star center point
    bool m_convex;         ///< controls if the star is convex
};

#endif /* KOSTARSHAPE_H */

