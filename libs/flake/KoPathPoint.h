/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KOPATHPOINT_H
#define KOPATHPOINT_H

#include <flake_export.h>

//#include <QMap>

//#include "KoShape.h"

class KoPathShape;
class QPointF;
class QMatrix;
class QRectF;
class QPainter;
class QSizeF;
class KoPointGroup;

//   typedef QMap<KoPathShape *, QSet<KoPathPoint *> > KoPathShapePointMap;
//   typedef QPair<int,int> KoPathPointIndex;
//   typedef QMap<KoPathShape *, QSet<KoPathPointIndex> > KoPathShapePointIndexMap;

/**
 * @brief A KoPathPoint represents a point in a path.
 *
 * A KoPathPoint stores a point in a path. Additional to this point 
 * 2 control points are stored. 
 * controlPoint1 is used to describe the second point of a cubic 
 * bezier ending at the point. controlPoint2 is used to describe the 
 * first point of a cubic bezier curve starting at the point.
 */
class FLAKE_EXPORT KoPathPoint
{
public:
    /// property enum
    enum KoPointProperty
    {
        Normal = 0, ///< it has no control points
        CanHaveControlPoint1 = 1, ///< it can have a control point 1
        CanHaveControlPoint2 = 2, ///< it can have a control point 2
        HasControlPoint1 = 4, ///< it has a control point 1
        HasControlPoint2 = 8, ///< it has a control point 2
        StartSubpath = 16, ///< it starts a new subpath by a moveTo command
        CloseSubpath = 32, ///< it closes a subpath
        IsSmooth = 64, ///< it is smooth, both control points on a line through the point
        IsSymmetric = 128 ///< it is symmetric, like smooth but control points have same distance to point
    };
    Q_DECLARE_FLAGS( KoPointProperties, KoPointProperty )

    /// the type for identifying part of a KoPathPoint
    enum KoPointType {
        Node = 1,          ///< the node point
        ControlPoint1 = 2, ///< the first control point
        ControlPoint2 = 4,  ///< the second control point
        All = 7
    };
    Q_DECLARE_FLAGS( KoPointTypes, KoPointType )

    /// Default constructor
    KoPathPoint();

    /**
     * @brief Constructor
     *
     * @param path is a pointer to the path shape this point is used in
     * @param point the position relative to the shape origin
     * @param properties describing the point
     */
    KoPathPoint( KoPathShape * path, const QPointF & point, KoPointProperties properties = Normal );

    /**
     * @brief Copy Constructor
     */
    KoPathPoint( const KoPathPoint & pathPoint );

    /**
     * @brief Assignment operator.
     */
    KoPathPoint& operator=( const KoPathPoint &rhs );

    /**
     * @brief Destructor
     */
    ~KoPathPoint();

    /**
     * @brief return the position relative to the shape origin
     *
     * @return point
     */
    QPointF point() const;

    /**
     * @brief get the control point 1
     *
     * This points is used for controlling a curve ending at this point
     *
     * @return control point 1 of this point
     */
    QPointF controlPoint1() const;

    /**
     * @brief get the second control point
     *
     * This points is used for controlling a curve starting at this point
     *
     * @return control point 2 of this point
     */
    QPointF controlPoint2() const;

    /**
     * @brief alter the point
     *
     * @param point to set
     */
    void setPoint( const QPointF & point );

    /**
     * @brief Set the control point 1
     *
     * @param point to set
     */
    void setControlPoint1( const QPointF & point );

    /**
     * @brief Set the control point 2
     *
     * @param point to set
     */
    void setControlPoint2( const QPointF & point );

    void removeControlPoint1() { /*TODO*/ }
    void removeControlPoint2() { /*TODO*/ }

    /**
     * @brief Get the properties of a point
     *
     * @return properties of the point
     */
    KoPointProperties properties() const;

    /**
     * @brief Set the properties of a point
     * @param properties the new properties
     */
    void setProperties( KoPointProperties properties );

    /**
     * @brief Sets a single property of a point.
     * @param property the property to set
     */
    void setProperty( KoPointProperty property );

    /**
     * @brief Removes a property from the point.
     * @param property the property to remove 
     */
    void unsetProperty( KoPointProperty property );

    /**
     * @brief check if there is a controlPoint1
     *
     * @return true when CanHaveControlPoint1 and HasControlPoint1 is set
     * return false otherwise
     */
    bool activeControlPoint1() const;

    /**
     * @brief check if there is a controlPoint2
     *
     * @return true when CanHaveControlPoint2 and HasControlPoint2 is set
     * return false otherwise
     */
    bool activeControlPoint2() const;

    /**
     * @brief apply matrix on the point
     *
     * This does a matrix multiplication on all points of the point
     * 
     * @param matrix which will be applied to all points 
     * @param mapGroup true when the matrix should be also applied to 
     *                 all points of the group the point belongs to
     */
    void map( const QMatrix &matrix, bool mapGroup = false );

    /**
     * Paints the path point with the actual brush and pen
     * @param painter used for painting the shape point
     * @param size the drawing size of the shape point
     * @param types the points which should be painted
     * @param active If true only the given active points are painted
     *               If false all given points are used.
     */
    //void paint(QPainter &painter, const QSizeF &size, bool selected );
    void paint(QPainter &painter, const QSizeF &size, KoPointTypes types, bool active = true );

    /**
     * @brief Sets the parent path shape.
     * @param parent the new parent path shape
     */
    void setParent( KoPathShape* parent );

    /**
     * @brief Get the path shape the point belongs to
     * @return the path shape the point belongs to
     */
    KoPathShape * parent() const;

    /**
     * @brief Get the bounding rect of the point.
     * 
     * This takes into account if there are controlpoints 
     *
     * @param active If true only the active points are used in calculation
     *               of the bounding rectangle. If false all points are used.
     *
     * @return bounding rect in document coordinates
     */
    QRectF boundingRect( bool active = true ) const;

    /**
     * @brief Reverses the path point.
     *
     * The control points are swapped and the point properties are adjusted.
     * The position dependent properties like StartSubpath and CloseSubpath
     * are not changed.
     */
    void reverse();
protected:
    friend class KoPointGroup;
    friend class KoPathShape;
    void removeFromGroup();
    void addToGroup( KoPointGroup *pointGroup );
    KoPointGroup * group();
private:
    class Private;
    Private * const d;
};

//   /// a KoSubpath contains a path from a moveTo until a close or a new moveTo
//   typedef QList<KoPathPoint *> KoSubpath;
//   typedef QList<KoSubpath *> KoSubpathList;
//   /// A KoPathSegment is a pair two neighboring KoPathPoints 
//   typedef QPair<KoPathPoint*,KoPathPoint*> KoPathSegment;
//   /// The position of a path point within a path shape
//   typedef QPair<KoSubpath*, int> KoPointPosition;

Q_DECLARE_OPERATORS_FOR_FLAGS( KoPathPoint::KoPointProperties )
Q_DECLARE_OPERATORS_FOR_FLAGS( KoPathPoint::KoPointTypes )

#endif
