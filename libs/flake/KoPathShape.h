/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPATHSHAPE_H
#define KOPATHSHAPE_H

#include <koffice_export.h>

#include <QFlags>
#include <QPainterPath>
#include <QSet>
#include <QList>

#include "KoShape.h"

#define KoPathShapeId "KoPathShape"

class KoPathShape;
class KoPointGroup;
class KoPathPoint;

typedef QMap<KoPathShape *, QSet<KoPathPoint *> > KoPathShapePointMap;
typedef QPair<int,int> KoPathPointIndex;
typedef QMap<KoPathShape *, QSet<KoPathPointIndex> > KoPathShapePointIndexMap;

/**
 * @brief Describe a KoPathPoint by a KoPathShape and its indices
 */
class KoPathPointData
{
public:
    /// contructor
    KoPathPointData( KoPathShape * pathShape, const KoPathPointIndex & pathPointIndex )
    : m_pathShape( pathShape )
    , m_pointIndex( pathPointIndex )
    {}

    /// operator used for sorting
    bool operator<( const KoPathPointData & other ) const 
    { 
        return m_pathShape < other.m_pathShape ||
               ( m_pathShape == other.m_pathShape && 
                 ( m_pointIndex.first < other.m_pointIndex.first ||
                   ( m_pointIndex.first == other.m_pointIndex.first &&
                     m_pointIndex.second < other.m_pointIndex.second ) ) );

    }
    /// path shape the path point belongs too
    KoPathShape *m_pathShape;
    /// position of the point in the path shape
    KoPathPointIndex m_pointIndex;
};

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
    KoPathPoint()
    : m_shape( 0 )
    , m_point( 0.0, 0.0 )
    , m_properties( Normal )
    , m_pointGroup( 0 )
    {}

    /**
     * @brief Constructor
     *
     * @param path is a pointer to the path shape this point is used in
     * @param point the position relative to the shape origin
     * @param properties describing the point
     */
    KoPathPoint( KoPathShape * path, const QPointF & point, KoPointProperties properties = Normal )
    : m_shape( path )
    , m_point( point )
    , m_properties( properties )
    , m_pointGroup( 0 )
    {}

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
    ~KoPathPoint() {}

    /**
     * @brief return the position relative to the shape origin
     *
     * @return point
     */
    QPointF point() const { return m_point; }

    /**
     * @brief get the control point 1
     *
     * This points is used for controlling a curve ending at this point
     *
     * @return control point 1 of this point
     */
    QPointF controlPoint1() const { return m_controlPoint1; }

    /**
     * @brief get the second control point
     *
     * This points is used for controlling a curve starting at this point
     *
     * @return control point 2 of this point
     */
    QPointF controlPoint2() const { return m_controlPoint2; }

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
    KoPointProperties properties() const { return m_properties; }

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
     * @return false otherwise
     */
    bool activeControlPoint1() const;

    /**
     * @brief check if there is a controlPoint2
     *
     * @return true when CanHaveControlPoint2 and HasControlPoint2 is set
     * @return false otherwise
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
    KoPathShape * parent() const { return m_shape; }

    /**
     * @brief Get the bounding rect of the point.
     * 
     * This takes into account if there are controlpoints 
     *
     * @param active If true only the active points are used in caluclation
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
    KoPointGroup * group() { return m_pointGroup; }
private:
    KoPathShape * m_shape;
    QPointF m_point;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;
    KoPointProperties m_properties;
    KoPointGroup * m_pointGroup;
};

/**
 * @brief A KoPointGroup represents points in a path that should be treated as one
 *
 * In svg it is possible when you use a close and the create a new subpath not using 
 * a moveTo that the new subpath starts at the same point as the last subpath. As 
 * every point can only have 2 control points we have this class to group points 
 * together which should be handled as one in e.g. a move. 
 */
class KoPointGroup
{
public:    
    KoPointGroup() {}
    ~KoPointGroup() {}

    /**
     * @brief Add a point to the group
     */
    void add( KoPathPoint * point );
    /**
     * @brief Remove a point from the group
     * 
     * This also remove the pointer to the group in the point.
     * When the second last point is removed from the group, the 
     * group removes also the last point and deletes itself.
     */
    void remove( KoPathPoint * point );

    void map( const QMatrix &matrix );

    /**
     * @brief get The point belonging to the group
     *
     * @return all points of the group
     */
    const QSet<KoPathPoint *> & points() const { return m_points; }

private:
    QSet<KoPathPoint *> m_points;
};

/// a KoSubpath contains a path from a moveTo until a close or a new moveTo
typedef QList<KoPathPoint *> KoSubpath;
typedef QList<KoSubpath *> KoSubpathList;
/// A KoPathSegment is a pair two neighboring KoPathPoints 
typedef QPair<KoPathPoint*,KoPathPoint*> KoPathSegment;
/// The position of a path point within a path shape
typedef QPair<KoSubpath*, int> KoPointPosition;
/**
 * @brief This is the base for all graphical objects.
 *
 * All graphical objects are based on this object e.g. lines, rectangulars, pies 
 * and so on.
 *
 * The KoPathShape uses KoPathPoint's to describe the path of the shape. 
 *
 * Here a short example:
 * 3 points connected by a curveTo's described by the following svg:
 * M 100,200 C 100,100 250,100 250,200 C 250,200 400,300 400,200.
 * 
 * This will be stored in 3 KoPathPoint's as 
 * The first point contains in 
 *       point 100,200 
 *       controlPoint2 100,100
 * The second point contains in
 *       point 250,200
 *       controlPoint1 250,100
 *       controlPoint2 250,300
 * The third point contains in
 *       point 400,300
 *       controlPoint1 400,200
 *       
 * Not the segments are stored but the points. Out of the points the segments are 
 * generated. See the outline method. The reason for storing it like that is that 
 * it is the points that are modified by the user and not the segments.
 */
class FLAKE_EXPORT KoPathShape : public KoShape
{
public:
    /**
     * @brief
     */
    KoPathShape();

    /**
     * @brief
     */
    virtual ~KoPathShape();

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void paintPoints( QPainter &painter, const KoViewConverter &converter, int handleRadius );
    virtual const QPainterPath outline() const;
    virtual QRectF boundingRect() const;
    virtual QSizeF size() const;
    virtual QPointF position() const;
    virtual void resize( const QSizeF &size );

    /// Removes all subpaths and their points from the path
    void clear();
    /**
     * @brief Start a new Subpath
     *
     * Moves the pen to p and starts a new subpath.
     *
     * @return The newly created point 
     */
    KoPathPoint * moveTo( const QPointF &p );

    /**
     * @brief add a line
     *
     * Adds a straight line between the last point and the given p.
     *
     * @return The newly created point 
     */
    KoPathPoint * lineTo( const QPointF &p );

    /**
     * @brief add a cubic Bezier curve.
     *
     * Adds a cubic Bezier curve between the last point and the given p,
     * using the control points specified by c1, and c2.
     * @param c1 control point1
     * @param c2 control point2
     * @param p The endpoint of this curve-part
     *
     * @return The newly created point 
     */
    KoPathPoint * curveTo( const QPointF &c1, const QPointF &c2, const QPointF &p );

    /**
     * @brief add a arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     *
     * @return The newly created point 
     */
    KoPathPoint * arcTo( double rx, double ry, double startAngle, double sweepAngle );

    /**
     * @brief close the current subpath
     */
    void close();

    /**
     * @brief close the current subpath
     *
     * It tries to merge the last and first point of the subpath
     * to one point and then closes the subpath. If merging is not 
     * possible as the two point are to far from each other a close
     * will be done.
     * TODO define a maximum distance between  the two points until this is working
     */
    void closeMerge();

    /**
     * @brief The path is updated
     *
     * This is called when a point of the path is updated. It will be used 
     * to make it possible to cache things.
     */
    void update();

    /**
     * @brief Normalizes the path data.
     *
     * The path points are transformed so that the top-left corner
     * of the bounding rect is (0,0).
     * This should be called after adding points to the path.
     * @return the offset by which the points are moved in shape coordinates.
     */
    virtual QPointF normalize();

    /**
     * @brief Returns the path points within the given rectangle.
     * @param r the rectangle the requested points are in
     * @return list of points within the rectangle
     */
    QList<KoPathPoint*> pointsAt( const QRectF &r );

    /**
     * @brief Get the path point index of a point
     *
     * @param point for which you want to get the info
     * @return path point index of the point if it exists
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex pathPointIndex( const KoPathPoint *point ) const;

    /**
     * @brief Get the point defined by a path point index
     *
     * @param pointIndex of the point to get
     *
     * @return KoPathPoint on success, 0 otherwise e.g. out of bounds
     */
    KoPathPoint * pointByIndex( const KoPathPointIndex &pointIndex ) const;

    /**
     * @brief Get the segment defined by a path point index
     *
     * A semgent is defined by the point index of the first point in the segment.
     * A segment contains the defined point and its following point. If the subpath is 
     * closed and the and the pointIndex point to the last point in the subpath, the 
     * following point is the first point in the subpath.
     *
     * @param pointIndex of the first point of the segment
     *
     * @return Segment containing both points of the segment or KoPathSegment( 0, 0 ) on error e.g. out of bounds
     */
    KoPathSegment segmentByIndex( const KoPathPointIndex &pointIndex ) const;

    /**
     * @brief Get the number of points in the path
     *
     * @return The number of points in the path
     */
    int pointCount() const;

    /**
     * @brief Get the number of subpaths in the path
     *
     * @return The number of subpaths in the path
     */
    int subpathCount() const;

    /**
     * @brief Get the number of points in a subpath
     *
     * @return The number of points in the subpath or -1 if subpath out of bounds
     */
    int pointCountSubpath( int subpathIndex ) const;

    /**
     * @brief Check if subpath is closed
     *
     * @param subpathIndex of the subpath to check
     *
     * @return true when the subpath is closed, false otherwise
     */
    bool isClosedSubpath( int subpathIndex );

    /**
     * @brief Inserts a new point into the given subpath at the specified position
     *
     * This method keeps the subpath closed if it is closed, and open when it was 
     * open. So it can change the properties of the point inserted.
     * You might need to update the point before/after to get the wanted result
     * e.g. when you insert the point into a curve.
     *
     * @param point to insert
     * @param pointIndex where the point should be inserted
     *
     * @return true on success, 
     *         false when pointIndex is out of bounds
     */
    bool insertPoint( KoPathPoint* point, const KoPathPointIndex &pointIndex );

    /**
     * @brief Removes point from the path.
     *
     * @param pointIndex of the point which should be removed
     *
     * @return The removed point on success,
     *         otherwise 0
     */
    KoPathPoint * removePoint( const KoPathPointIndex &pointIndex );

    /**
     * @brief Breaks the path after the point index
     *
     * The new subpath will be behind the one that was broken. The segment between 
     * the given point and the one behind will be removed. If you want to split at 
     * one point insert first a copy of the point behind it.
     * This does not work when the subpath is closed. Use openSubpath for this.
     * It does not break at the last position of a subpath or if there is only one 
     * point in the subpath.
     *
     * @param pointIndex after which the path should be broken
     *
     * @return true if the subpath was broken, otherwise false
     */
    bool breakAfter( const KoPathPointIndex &pointIndex );

    /**
     * @brief Joins the given subpath with the following one
     *
     * Joins the given subpath with the following one by inserting a segment between 
     * the two subpathes.
     * This does nothing if the given ot the following subpath is closed or on the 
     * last subpath of the path.
     * 
     * @param subpathIndex of the subpath which will be joined with its following one
     *
     * @return true if the subpath was joined, otherwise false
     */
    bool join( int subpathIndex );

    /**
     * @brief Move the position of a subpath within a path
     *
     * @param oldSubpathIndex old index of the subpath
     * @param newSubpathIndex new index of the subpath
     *
     * @return true if the subpath was moved, otherwise false e.g. if an index is out of bounds
     */
    bool moveSubpath( int oldSubpathIndex, int newSubpathIndex );

    /**
     * @brief Open a closed subpath
     *
     * The subpath is opened by removing the segment before the given point, makeing
     * the given point the new start point of the subpath.
     *
     * @param pointIndex to where to open the closed subpath
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex openSubpath( const KoPathPointIndex &pointIndex );

    /**
     * @brief Close a open subpath
     *
     * The subpath is closed be inserting a segment between the start and end point, makeing
     * the given point the new start point of the subpath.
     *
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex closeSubpath( const KoPathPointIndex &pointIndex );

    /**
     * @brief Reverse subpath 
     *
     * The last point becomes the first point and the first one becomes the last one.
     *
     * @param subpathIndex the pointIndex of the subpath to reverse
     */
    bool reverseSubpath( int subpathIndex );

    /**
     * @brief Remove subpath from path
     *
     * @return The removed subpath on succes, 0 otherwise.
     */
    KoSubpath * removeSubpath( int subpathIndex );

    /**
     * @brief Add a Subpath at the given index to the path
     *
     * @return true on success, false otherwise e.g. subpathIndex out of bounds
     */
    bool addSubpath( KoSubpath * subpath, int subpathIndex );

    /**
     * @brief Combines two path by appending the data of the specified path.
     * @param path the path to combine with
     * @return true if combining was successful, else false
     */
    bool combine( KoPathShape *path );

    /**
     * @brief Creates separate path shapes, one for each existing subpath.
     * @param separatedPaths the list which contains the separated path shapes
     * @return true if separating the path was successful, else false 
     */
    bool separate( QList<KoPathShape*> & separatedPaths );

    /**
     * @brief print debug information about a the points of the path
     */
    void debugPath();

    /**
     * @brief transform point from shape coordinates to document coordinates
     *
     * @param point in shape coordinates
     *
     * @return point in document coordinates
     */
    QPointF shapeToDocument( const QPointF &point ) const;
    
    /**
     * @brief transform rect from shape coordinates to document coordinates
     *
     * @param rect in shape coordinates
     *
     * @return rect in document coordinates
     */
    QRectF shapeToDocument( const QRectF &rect ) const;
    
    /**
     * @brief transform point from world coordinates to document coordinates
     *
     * @param point in document coordinates
     *
     * @return point in shape coordinates
     */
    QPointF documentToShape( const QPointF &point ) const;
    
    /**
     * @brief transform rect from world coordinates to document coordinates
     *
     * @param rect in document coordinates
     *
     * @return rect in shape coordinates
     */
    QRectF documentToShape( const QRectF &rect ) const;

private:
    void map( const QMatrix &matrix );

    void updateLast( KoPathPoint ** lastPoint );

    /// closes specified subpath
    void closeSubpath( KoSubpath *subpath );
    /// close-merges specified subpath
    void closeMergeSubpath( KoSubpath *subpath );

    /**
     * @brief Get subpath at subpathIndex
     *
     * @return subPath on success, or 0 when subpathIndex is out of bounds
     */
    KoSubpath * subPath( int subpathIndex ) const;
#ifndef NDEBUG
    void paintDebug( QPainter &painter );
#endif

protected:    
    QRectF handleRect( const QPointF &p, double radius ) const;
    /**
     * @brief add a arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     * @param offset to the first point in the arc
     * @param curvePoints a array which take the cuve points, pass a 'QPointF curvePoins[12]';
     *
     * @return number of points created by the curve
     */
    int arcToCurve( double rx, double ry, double startAngle, double sweepAngle, const QPointF & offset, QPointF * curvePoints ) const;

    friend class KoParameterToPathCommand;

    KoSubpathList m_subpaths;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KoPathPoint::KoPointProperties )
Q_DECLARE_OPERATORS_FOR_FLAGS( KoPathPoint::KoPointTypes )

#endif /* KOPATHSHAPE_H */
