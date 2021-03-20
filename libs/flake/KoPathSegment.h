/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHSEGMENT_H
#define KOPATHSEGMENT_H

#include "kritaflake_export.h"
#include <QList>
#include <QPair>

class KoPathPoint;
class QTransform;
class QPointF;
class QRectF;

/// A KoPathSegment consist of two neighboring KoPathPoints
class KRITAFLAKE_EXPORT KoPathSegment
{
public:
    /**
    * Creates a new segment from the given path points
    * It takes ownership of the path points which do not have a
    * parent path shape set.
    */
    explicit KoPathSegment(KoPathPoint * first = 0, KoPathPoint * second = 0);

    /// Constructs segment by copying another segment
    KoPathSegment(const KoPathSegment &segment);

    /// Creates a new line segment
    KoPathSegment(const QPointF &p0, const QPointF &p1);
    /// Creates a new quadratic segment
    KoPathSegment(const QPointF &p0, const QPointF &p1, const QPointF &p2);
    /// Creates a new cubic segment
    KoPathSegment(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3);

    /// Assigns segment
    KoPathSegment& operator=(const KoPathSegment &other);

    /// Destroys the path segment
    ~KoPathSegment();

    /// Returns the first point of the segment
    KoPathPoint *first() const;

    /// Sets the first segment point
    void setFirst(KoPathPoint *first);

    /// Returns the second point of the segment
    KoPathPoint *second() const;

    /// Sets the second segment point
    void setSecond(KoPathPoint *second);

    /// Returns if segment is valid, e.g. has two valid points
    bool isValid() const;

    /// Compare operator
    bool operator==(const KoPathSegment &other) const;

    /// Returns the degree of the segment: 1 = line, 2 = quadratic, 3 = cubic, -1 = invalid
    int degree() const;

    /// Returns list of intersections with the given path segment
    QList<QPointF> intersections(const KoPathSegment &segment) const;

    /// Returns the convex hull polygon of the segment
    QList<QPointF> convexHull() const;

    /// Splits segment at given position returning the two resulting segments
    QPair<KoPathSegment, KoPathSegment> splitAt(qreal t) const;

    /// Returns point at given t
    QPointF pointAt(qreal t) const;

    /// Returns the axis aligned tight bounding rect
    QRectF boundingRect() const;

    /// Returns the control point bounding rect
    QRectF controlPointRect() const;

    /// Returns transformed segment
    KoPathSegment mapped(const QTransform &matrix) const;

    /// Returns cubic bezier curve segment of this segment
    KoPathSegment toCubic() const;

    /**
     * Returns the length of the path segment
     * @param error the error tolerance
     */
    qreal length(qreal error = 0.005) const;

    /**
     * Returns segment length at given parameter
     *
     * Splits the segment at the given parameter t and calculates
     * the length of the first segment of the split.
     *
     * @param t curve parameter to get length at
     * @param error the error tolerance
     */
    qreal lengthAt(qreal t, qreal error = 0.005) const;

    /**
     * Returns the curve parameter at the given length of the segment
     *
     * If the specified length is negative it returns 0.0.
     * If the specified length is bigger that the actual length of the
     * segment it return 1.0.
     *
     * @param length the length to get the curve parameter for
     * @param tolerance the length error tolerance
     */
    qreal paramAtLength(qreal length, qreal tolerance = 0.001) const;

    /**
     * Checks if the segment is flat, i.e. the height smaller then the given tolerance
     * @param tolerance the flatness tolerance
     */
    bool isFlat(qreal tolerance = 0.01) const;

    /**
     * Returns the parameter for the curve point nearest to the given point
     * @param point the point to find nearest point to
     * @return the parameter of the curve point nearest to the given point
     */
    qreal nearestPoint(const QPointF &point) const;

    /// Returns ordered list of control points
    QList<QPointF> controlPoints() const;

    /**
     * Interpolates quadric bezier curve.
     * @return the interpolated bezier segment
     */
    static KoPathSegment interpolate(const QPointF &p0, const QPointF &p1, const QPointF &p2, qreal t);

private:
    class Private;
    Private * const d;
};

#endif // KOPATHSEGMENT_H
