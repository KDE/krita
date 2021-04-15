/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOELLIPSESHAPE_H
#define KOELLIPSESHAPE_H

#include "KoParameterShape.h"
#include <SvgShape.h>

#define EllipseShapeId "EllipseShape"

/**
 * This class adds support for arc, pie, chord, circle and ellipse
 * shapes. The ellipse/circle radii are defined by the actual size
 * of the ellipse shape which can be changed with the setSize
 * method.
 */
class EllipseShape : public KoParameterShape, public SvgShape
{
public:
    /// the possible ellipse types
    enum EllipseType {
        Arc = 0,   ///< an ellipse arc
        Pie = 1,   ///< an ellipse pie
        Chord = 2  ///< an ellipse chord
    };

    EllipseShape();
    ~EllipseShape() override;

    KoShape* cloneShape() const override;

    void setSize(const QSizeF &newSize) override;
    QPointF normalize() override;

    /**
     * Sets the type of the ellipse.
     * @param type the new ellipse type
     */
    void setType(EllipseType type);

    /// Returns the actual ellipse type
    EllipseType type() const;

    /**
     * Sets the start angle of the ellipse.
     * @param angle the new start angle in degree
     */
    void setStartAngle(qreal angle);

    /// Returns the actual ellipse start angle in degree
    qreal startAngle() const;

    /**
     * Sets the end angle of the ellipse.
     * @param angle the new end angle in degree
     */
    void setEndAngle(qreal angle);

    /// Returns the actual ellipse end angle in degree
    qreal endAngle() const;

    /// reimplemented
    QString pathShapeId() const override;

    /// reimplemented from SvgShape
    bool saveSvg(SvgSavingContext &context) override;

    /// reimplemented from SvgShape
    bool loadSvg(const QDomElement &element, SvgLoadingContext &context) override;

protected:

    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;
    void updatePath(const QSizeF &size) override;
    void createPoints(int requiredPointCount);

private:
    qreal sweepAngle() const;

    void updateKindHandle();
    void updateAngleHandles();

    EllipseShape(const EllipseShape &rhs);

    // start angle in degree
    qreal m_startAngle;
    // end angle in degree
    qreal m_endAngle;
    // angle for modifying the kind in radiant
    qreal m_kindAngle;
    // the center of the ellipse
    QPointF m_center;
    // the radii of the ellipse
    QPointF m_radii;
    // the actual ellipse type
    EllipseType m_type;
};

#endif /* KOELLIPSESHAPE_H */

