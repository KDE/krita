/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef SVGMESHPATCH_H
#define SVGMESHPATCH_H

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QMap>
#include <QScopedPointer>
#include <QPainterPath>

#include <KoPathShape.h>

struct SvgMeshStop {
    QColor color;
    QPointF point;

    SvgMeshStop()
    {}

    SvgMeshStop(QColor color, QPointF point)
        : color(color), point(point)
    {}

    bool isValid() const { return color.isValid(); }
};


class SvgMeshPatch
{
public:
    /// Position of stop in the patch
    enum Type {
        Top = 0,
        Right,
        Bottom,
        Left,
        Size,
    };

    SvgMeshPatch(QPointF startingPoint);
    SvgMeshPatch(const SvgMeshPatch& other);

    // NOTE: NO path is created here
    // sets a new starting point for the patch
    void moveTo(const QPointF& p);
    /// Helper to convert to a cubic curve internally.
    void lineTo(const QPointF& p);
    /// add points as curve.
    void curveTo(const QPointF& c1, const QPointF& c2, const QPointF& p);

    /// returns the starting point of the stop
    SvgMeshStop getStop(Type type) const;

    /// returns the midPoint in parametric space
    inline QPointF getMidpointParametric(Type type) const {
        return (m_parametricCoords[type] + m_parametricCoords[(type + 1) % Size]) * 0.5;
    }

    /// get the point on a segment using De Casteljau's algorithm
    QPointF segmentPointAt(Type type, qreal t) const;

    /// split a segment using De Casteljau's algorithm
    QPair<std::array<QPointF, 4>, std::array<QPointF, 4>> segmentSplitAt(Type type, qreal t) const;

    /// Get a segment of the path in the meshpatch
    std::array<QPointF, 4> getSegment(Type type) const;

    /// Get full (closed) meshpath
    QPainterPath getPath() const;

    /// Get size swept by mesh in pts
    QSizeF size() const;

    QRectF boundingRect() const;

    /// Gets the curve passing through the middle of meshpatch
    std::array<QPointF, 4> getMidCurve(bool isVertical) const;

    void subdivide(QVector<SvgMeshPatch*>& subdivided, const QVector<QColor>& colors) const;

    int countPoints() const;

    /* Parses raw pathstr and adds path to the shape, if the path isn't
     * complete, it will have to be computed and given with pathIncomplete = true
     * (Ideal case for std::optional)
     */
    void addStop(const QString& pathStr, QColor color, Type edge, bool pathIncomplete = false, QPointF lastPoint = QPointF());

    /// Adds path to the shape
    void addStop(const std::array<QPointF, 4>& pathPoints, QColor color, Type edge);

    /// Adds linear path to the shape
    void addStopLinear(const std::array<QPointF, 2>& pathPoints, QColor color, Type edge);

    void setTransform(const QTransform& matrix);

private:
    /* Parses path and adds it to m_path and returns the last point of the curve/line
     * see also: SvgMeshPatch::addStop
     */
    QPointF parseMeshPath(const QString& path, bool pathIncomplete = false, const QPointF lastPoint = QPointF());
    const char* getCoord(const char* ptr, qreal& number);

private:
    bool m_newPath;
    int counter {0};

    /// This is the starting point for each path
    QPointF m_startingPoint;

    std::array<SvgMeshStop, Size> m_nodes;
    std::array<std::array<QPointF, 4>, 4> controlPoints;
    /// Coordinates in UV space
    std::array<QPointF, 4> m_parametricCoords;
};

#endif // SVGMESHPATCH_H
