/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>

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

#include "flake_export.h"

class KoPathShape;
class QPointF;
class QTransform;
class QRectF;
class QPainter;
class KoPointGroup;

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
    enum PointProperty {
        Normal = 0, ///< it has no control points
        StartSubpath = 1, ///< it starts a new subpath by a moveTo command
        StopSubpath = 2, ///< it stops a subpath (last point of subpath)
        CloseSubpath = 8, ///< it closes a subpath (only applicable on StartSubpath and StopSubpath)
        IsSmooth = 16, ///< it is smooth, both control points on a line through the point
        IsSymmetric = 32 ///< it is symmetric, like smooth but control points have same distance to point
    };
    Q_DECLARE_FLAGS(PointProperties, PointProperty)

    /// the type for identifying part of a KoPathPoint
    enum PointType {
        Node = 1,          ///< the node point
        ControlPoint1 = 2, ///< the first control point
        ControlPoint2 = 4,  ///< the second control point
        All = 7
    };
    Q_DECLARE_FLAGS(PointTypes, PointType)

    /// Default constructor
    KoPathPoint();

    /**
     * @brief Constructor
     *
     * @param path is a pointer to the path shape this point is used in
     * @param point the position relative to the shape origin
     * @param properties describing the point
     */
    KoPathPoint(KoPathShape *path, const QPointF &point, PointProperties properties = Normal);

    /**
     * @brief Copy Constructor
     */
    KoPathPoint(const KoPathPoint &pathPoint);

    /**
     * @brief Assignment operator.
     */
    KoPathPoint& operator=(const KoPathPoint &other);

    /// Compare operator
    bool operator == (const KoPathPoint &other) const;

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
    void setPoint(const QPointF &point);

    /**
     * @brief Set the control point 1
     *
     * @param point to set
     */
    void setControlPoint1(const QPointF &point);

    /**
     * @brief Set the control point 2
     *
     * @param point to set
     */
    void setControlPoint2(const QPointF &point);

    /// Removes the first control point
    void removeControlPoint1();

    /// Removes the second control point
    void removeControlPoint2();

    /**
     * @brief Get the properties of a point
     *
     * @return properties of the point
     */
    PointProperties properties() const;

    /**
     * @brief Set the properties of a point
     * @param properties the new properties
     */
    void setProperties(PointProperties properties);

    /**
     * @brief Sets a single property of a point.
     * @param property the property to set
     */
    void setProperty(PointProperty property);

    /**
     * @brief Removes a property from the point.
     * @param property the property to remove
     */
    void unsetProperty(PointProperty property);

    /**
     * @brief Checks if there is a controlPoint1
     *
     * The control point is active if a control point was set by
     * calling setControlPoint1. However a start point of a subpath
     * (StartSubpath) can only have an active control 1 if the
     * subpath is closed (CloseSubpath on first and last point).
     *
     * @return true if control point is active, false otherwise
     */
    bool activeControlPoint1() const;

    /**
     * @brief Checks if there is a controlPoint2
     *
     * The control point is active if a control point was set by
     * calling setControlPoint2. However a end point of a subpath
     * (StopSubpath) can only have an active control point 2 if there
     * subpath is closed (CloseSubpath on first and last point).
     *
     * @return true if control point is active, false otherwise
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
    void map(const QTransform &matrix, bool mapGroup = false);

    /**
     * Paints the path point with the actual brush and pen
     * @param painter used for painting the shape point
     * @param handleRadius size of point handles in pixel
     * @param types the points which should be painted
     * @param active If true only the given active points are painted
     *               If false all given points are used.
     */
    void paint(QPainter &painter, int handleRadius, PointTypes types, bool active = true);

    /**
     * @brief Sets the parent path shape.
     * @param parent the new parent path shape
     */
    void setParent(KoPathShape* parent);

    /**
     * @brief Get the path shape the point belongs to
     * @return the path shape the point belongs to
     */
    KoPathShape *parent() const;

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
    QRectF boundingRect(bool active = true) const;

    /**
     * @brief Reverses the path point.
     *
     * The control points are swapped and the point properties are adjusted.
     * The position dependent properties like StartSubpath and CloseSubpath
     * are not changed.
     */
    void reverse();

    /**
     * Returns if this point is a smooth join of adjacent path segments.
     *
     * The smoothess is defined by the parallelness of the tangents emanating
     * from the knot point, i.e. the normalized vectors from the knot to the
     * first and second control point.
     * The previous and next path points are used to determine the smoothness
     * in case this path point has not two control points.
     *
     * @param previous the previous path point
     * @param next the next path point
     */
    bool isSmooth(KoPathPoint *previous, KoPathPoint *next) const;

protected:
    friend class KoPointGroup;
    friend class KoPathShape;
    friend class KoPathShapePrivate;
    void removeFromGroup();
    void addToGroup(KoPointGroup *pointGroup);
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

Q_DECLARE_OPERATORS_FOR_FLAGS(KoPathPoint::PointProperties)
Q_DECLARE_OPERATORS_FOR_FLAGS(KoPathPoint::PointTypes)

#endif
